#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

using namespace std;

// Установка неблокирующего режима
bool setNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) != -1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Использование: " << argv[0] << " <ip_сервера> [порт]\n";
        cerr << "Пример: " << argv[0] << " localhost 6379\n";
        return 1;
    }
    
    string server_ip = argv[1];
    int port = (argc > 2) ? stoi(argv[2]) : 6379;
    
    if (server_ip == "localhost") {
        server_ip = "127.0.0.1";
    }
    
    // Создаем сокет
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Не удалось создать сокет\n";
        return 1;
    }
    
    // Настраиваем адрес сервера
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        cerr << "Неверный IP адрес: " << server_ip << "\n";
        close(sock);
        return 1;
    }
    
    // Подключаемся к серверу
    cout << "Подключение к " << server_ip << ":" << port << "...\n";
    
    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Не удалось подключиться\n";
        close(sock);
        return 1;
    }
    
    cout << "Подключение успешно!\n";
    
    // Создаем структуру для poll
    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN;
    
    char buffer[1024];
    
    // Читаем приветственное сообщение с таймаутом
    int timeout = 5000;
    int ret = poll(fds, 1, timeout);
    
    if (ret > 0 && (fds[0].revents & POLLIN)) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            cout << buffer;
        }
    }
    
    // Очищаем буфер сокета от возможных остаточных данных
    setNonBlocking(sock);
    
    // Пытаемся прочитать все, что есть в буфере
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
        if (bytes <= 0) {
            break;
        }
    }
    
    // Возвращаем блокирующий режим
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
    
    cout << "Вводите команды (для выхода введите 'exit' или 'quit'):\n";
    
    while (true) {
        cout << "> ";
        string command;
        getline(cin, command);
        
        if (command == "exit" || command == "quit") {
            command += "\n";
            send(sock, command.c_str(), command.length(), 0);
            
            // Ждем прощальное сообщение
            memset(buffer, 0, sizeof(buffer));
            recv(sock, buffer, sizeof(buffer) - 1, 0);
            cout << buffer;
            break;
        }
        
        if (command.empty()) {
            continue;
        }
        
        // Отправляем команду серверу
        command += "\n";
        int sent = send(sock, command.c_str(), command.length(), 0);
        if (sent < 0) {
            cerr << "Ошибка отправки\n";
            break;
        }
        
        // Ждем ответ с таймаутом
        timeout = 3000;
        ret = poll(fds, 1, timeout);
        
        if (ret > 0 && (fds[0].revents & POLLIN)) {
            // Читаем ответ
            memset(buffer, 0, sizeof(buffer));
            int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes > 0) {
                buffer[bytes] = '\0';

                string response(buffer);
                if (!response.empty() && response.back() == '\n') {
                    response.pop_back();
                }
                if (!response.empty() && response.back() == '\r') {
                    response.pop_back();
                }
                
                cout << response << endl;
            } else if (bytes == 0) {
                cout << "Сервер отключился\n";
                break;
            } else {
                cerr << "Ошибка при получении данных\n";
                break;
            }
        } else if (ret == 0) {
            cout << "Таймаут ожидания ответа\n";
        } else {
            cerr << "Ошибка poll\n";
            break;
        }
    }
    
    close(sock);
    cout << "Соединение закрыто\n";
    
    return 0;
}
