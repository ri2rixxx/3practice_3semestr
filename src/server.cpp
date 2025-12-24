#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "database.h"

using namespace std;

mutex db_mutex;
Database global_db;
string db_filename;

void handleClient(int client_fd, string client_ip, int client_port) {
    cout << "Клиент подключен: " << client_ip << ":" << client_port << endl;
    
    char buffer[1024];
    
    string welcome = "Добро пожаловать в СУБД сервер!\n";
    welcome += "Доступные команды:\n";
    welcome += "  HSET <таблица> <ключ> <значение>  - добавить в хеш-таблицу\n";
    welcome += "  HGET <таблица> <ключ>             - получить из хеш-таблицы\n";
    welcome += "  HDEL <таблица> <ключ>             - удалить из хеш-таблицы\n";
    welcome += "  SPUSH <стек> <значение>           - добавить в стек\n";
    welcome += "  SPOP <стек>                       - извлечь из стека\n";
    welcome += "  QPUSH <очередь> <значение>        - добавить в очередь\n";
    welcome += "  QPOP <очередь>                    - извлечь из очереди\n";
    welcome += "  SADD <множество> <значение>       - добавить в множество\n";
    welcome += "  SREM <множество> <значение>       - удалить из множества\n";
    welcome += "  SISMEMBER <множество> <значение>  - проверить наличие\n";
    welcome += "  exit или quit                     - выйти\n\n";
    send(client_fd, welcome.c_str(), welcome.length(), 0);
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        
        // Читаем команду от клиента
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes <= 0) {
            cout << "Клиент отключился: " << client_ip << ":" << client_port << endl;
            break;
        }
        
        // Преобразуем в строку
        string command(buffer);

        if (!command.empty() && command.back() == '\n') {
            command.pop_back();
        }
        if (!command.empty() && command.back() == '\r') {
            command.pop_back();
        }
        
        cout << "Команда от " << client_ip << ":" << client_port << ": " << command << endl;
        
        // Выход
        if (command == "exit" || command == "quit") {
            string response = "До свидания!\n";
            send(client_fd, response.c_str(), response.length(), 0);
            break;
        }
        
        // Пустая команда
        if (command.empty()) {
            send(client_fd, "\n", 1, 0);
            continue;
        }
        
        // Обрабатываем команду
        string result;
        {
            lock_guard<mutex> lock(db_mutex);
            result = executeQuery(global_db, command);
            saveDatabase(global_db, db_filename);
        }
        
        // Отправляем результат клиенту
        result += "\n";
        int sent_bytes = send(client_fd, result.c_str(), result.length(), 0);
        
        cout << "Ответ клиенту " << client_ip << ":" << client_port << " (" << sent_bytes << " байт): " << result;
    }
    
    close(client_fd);
    cout << "Соединение закрыто: " << client_ip << ":" << client_port << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Использование: " << argv[0] << " --file <файл> [--port <порт>]\n";
        cerr << "Пример: " << argv[0] << " --file data.db --port 6379\n";
        return 1;
    }
    
    string filename;
    int port = 6379;
    
    // Парсинг аргументов
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--file" && i + 1 < argc) {
            filename = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = stoi(argv[++i]);
        }
    }
    
    if (filename.empty()) {
        cerr << "Ошибка: Укажите файл базы данных с помощью --file\n";
        return 1;
    }
    
    db_filename = filename;
    
    try {
        // Инициализация БД
        initDatabase(global_db);
        loadDatabase(global_db, filename);
        
        cout << "База данных загружена из: " << filename << endl;
        
        // Создаем TCP сокет
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            throw runtime_error("Не удалось создать сокет");
        }
        
        // Разрешаем переиспользование порта
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(server_fd);
            throw runtime_error("Не удалось установить опции сокета");
        }
        
        // Настраиваем адрес сервера
        sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        // Привязываем сокет к порту
        if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(server_fd);
            throw runtime_error("Не удалось привязаться к порту " + to_string(port));
        }
        
        // Начинаем слушать подключения
        if (listen(server_fd, 10) < 0) {
            close(server_fd);
            throw runtime_error("Не удалось начать прослушивание");
        }
        
        cout << "Сервер СУБД запущен на порту " << port << endl;
        cout << "Ожидание подключений...\n";
        
        // Основной цикл сервера
        while (true) {
            sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            // Принимаем подключение
            int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                cerr << "Не удалось принять подключение\n";
                continue;
            }
            
            // Получаем IP клиента
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
            int client_port = ntohs(client_addr.sin_port);
            
            // Запускаем обработчик в отдельном потоке
            thread(handleClient, client_fd, string(client_ip), client_port).detach();
        }
        
        // Закрываем сервер
        close(server_fd);
        freeDatabase(global_db);
        
    } catch (const exception& e) {
        cerr << "Критическая ошибка: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
