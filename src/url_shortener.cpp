#include "db_client.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <random>
#include <algorithm>

const std::string HASH_TABLE_NAME = "urls";
const int SHORT_CODE_LENGTH = 6;

class StringLogger {
private:
    std::ostringstream buffer;
    
public:
    void log(const std::string& message) {
        buffer << message << "\n";
    }
    
    void logIteration(int num, int index, char ch, const std::string& type, 
                     const std::string& formula, const std::string& code) {
        buffer << "Итерация" << num << "\n";
        buffer << "  Индекс: " << index << "\n";
        buffer << "  Тип: " << type << "\n";
        buffer << "  Формула: " << formula << "\n";
        buffer << "  Символ: '" << ch << "' (ASCII " << (int)ch << ")\n";
        buffer << "  Код: \"" << code << "\"\n\n";
    }
    
    std::string getLog() const {
        return buffer.str();
    }
    
    void writeToFile(const std::string& filename) {
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << buffer.str();
            file.close();
        }
    }
    
    void clear() {
        buffer.str("");
        buffer.clear();
    }
};

std::string generateShortCode() {
    StringLogger logger;
    
    logger.log("Генерация короткого кода");
    
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, 61);
    
    logger.log("Инициализация:");
    logger.log(" Генератор: MT19937 (Mersenne Twister)");
    logger.log(" Диапазон: 0-61 (62 символа)");
    logger.log(" Длина: " + std::to_string(SHORT_CODE_LENGTH) + " символов");
    logger.log("");
    logger.log(" 0-9   - Цифры 0-9      (ASCII 48-57)");
    logger.log(" 10-35 - Заглавные A-Z  (ASCII 65-90)");
    logger.log(" 36-61 - Строчные a-z   (ASCII 97-122)");
    logger.log("");
    
    std::string code;
    code.reserve(SHORT_CODE_LENGTH);
    
    for (int i = 0; i < SHORT_CODE_LENGTH; ++i) {
        int random_index = distribution(generator);
        
        char ch;
        std::string type, formula;
        
        if (random_index < 10) {
            ch = 48 + random_index;
            type = "цифра";
            formula = "48 + " + std::to_string(random_index) + " = " + std::to_string((int)ch);
        } else if (random_index < 36) {
            ch = 65 + (random_index - 10);
            type = "заглавная буква";
            formula = "65 + (" + std::to_string(random_index) + " - 10) = " + std::to_string((int)ch);
        } else {
            ch = 97 + (random_index - 36);
            type = "строчная буква";
            formula = "97 + (" + std::to_string(random_index) + " - 36) = " + std::to_string((int)ch);
        }
        
        code += ch;
        logger.logIteration(i + 1, random_index, ch, type, formula, code);
    }
    
    logger.log("Результат: \"" + code + "\"");
    logger.log("");
    
    // Записываем в файл
    logger.writeToFile("generation.log");
    
    return code;
}

void sendResponse(int client_sock, const std::string& status, 
                  const std::string& content_type, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    
    std::string resp_str = response.str();
    send(client_sock, resp_str.c_str(), resp_str.length(), 0);
}

void handleRequest(int client_sock, DBClient& db_client, 
                   struct sockaddr_in& client_addr, int http_port) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    
    ssize_t bytes_read = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        return;
    }
    
    std::string request(buffer);
    std::istringstream iss(request);
    std::string method, path, version;
    iss >> method >> path >> version;
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);
    
    std::cout << "REQUEST" << std::endl;
    std::cout << "From: " << client_ip << ":" << client_port << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "Path: " << path << std::endl;

    if (method == "POST" && path == "/") {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start == std::string::npos) {
            std::cout << "ERROR: No body found" << std::endl;
            sendResponse(client_sock, "400 Bad Request", "text/plain", "No body");
            return;
        }
        
        std::string body = request.substr(body_start + 4);
        std::string long_url = body;

        long_url.erase(std::remove(long_url.begin(), long_url.end(), '\r'), long_url.end());
        long_url.erase(std::remove(long_url.begin(), long_url.end(), '\n'), long_url.end());
        
        std::cout << "Long URL: [" << long_url << "]" << std::endl;
        
        if (long_url.empty()) {
            std::cout << "ERROR: Empty URL" << std::endl;
            sendResponse(client_sock, "400 Bad Request", "text/plain", "Empty URL");
            return;
        }
        
        std::string short_code = generateShortCode();
        std::string command = "HSET " + HASH_TABLE_NAME + " " + short_code + " " + long_url;
        std::string response = db_client.sendCommand(command);
        
        std::cout << "Created: " << short_code << " -> " << long_url << std::endl;
        
        std::string full_url = "http://localhost:" + std::to_string(http_port) + "/" + short_code;
        std::cout << "Returning: " << full_url << std::endl;
        sendResponse(client_sock, "200 OK", "text/plain", full_url);

    } else if (method == "GET" && path.length() > 1) {
        std::string short_code = path.substr(1);
        std::string command = "HGET " + HASH_TABLE_NAME + " " + short_code;
        
        std::cout << "Looking up: " << short_code << std::endl;
        
        std::string long_url = db_client.sendCommand(command);
        
        std::cout << "DB Response: [" << long_url << "]" << std::endl;
        
        if (long_url.empty() || long_url == "(nil)") {
            std::cout << "ERROR: Not found: " << short_code << std::endl;
            sendResponse(client_sock, "404 Not Found", "text/plain", "URL not found");
            return;
        }
        
        std::cout << "Redirecting to: " << long_url << std::endl;
        
        std::ostringstream redirect_response;
        redirect_response << "HTTP/1.1 301 Moved Permanently\r\n";
        redirect_response << "Location: " << long_url << "\r\n";
        redirect_response << "Connection: close\r\n\r\n";
        
        std::string resp = redirect_response.str();
        send(client_sock, resp.c_str(), resp.length(), 0);
        
    } else {
        std::cout << "ERROR: Unknown request" << std::endl;
        sendResponse(client_sock, "404 Not Found", "text/plain", "Not Found");
    }
    
    std::cout << "---------------" << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    int http_port = 8080;
    std::string db_host = "127.0.0.1";
    int db_port = 6379;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            http_port = std::stoi(argv[++i]);
        } else if (arg == "--db-host" && i + 1 < argc) {
            db_host = argv[++i];
        } else if (arg == "--db-port" && i + 1 < argc) {
            db_port = std::stoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "  --port <port>        HTTP port (default: 8080)\n"
                      << "  --db-host <host>     Database host (default: 127.0.0.1)\n"
                      << "  --db-port <port>     Database port (default: 6379)\n"
                      << "  --help               Show this help\n";
            return 0;
        }
    }
    
    DBClient db_client(db_host, db_port);
    
    try {
        db_client.connect();
        std::cout << "Connected to database " << db_host << ":" << db_port << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Database connection error: " << e.what() << std::endl;
        return 1;
    }
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(http_port);
    
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind error" << std::endl;
        return 1;
    }
    
    if (listen(server_sock, 10) < 0) {
        std::cerr << "Listen error" << std::endl;
        return 1;
    }
    
    std::cout << "URL Shortener running on port " << http_port << std::endl;
    std::cout << "Waiting for connections..." << std::endl << std::endl;
    
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            std::cerr << "Accept error" << std::endl;
            continue;
        }
        
        handleRequest(client_sock, db_client, client_addr, http_port);
        
        close(client_sock);
    }
    
    close(server_sock);
    return 0;
}
