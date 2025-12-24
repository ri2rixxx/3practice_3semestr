#include <iostream>
#include <string>
#include <vector>
#include "database.h"

using namespace std;

void printUsage() {
    cout << "Использование: ./dbms --file <имя_файла>" << endl;
    cout << endl;
    cout << "После запуска вводите команды в интерактивном режиме." << endl;
    cout << "Для выхода введите 'exit' или 'quit'." << endl;
    cout << endl;
    cout << "Примеры команд:" << endl;
    cout << "  HSET myhash key value" << endl;
    cout << "  HGET myhash key" << endl;
    cout << "  HDEL myhash key" << endl;
    cout << "  SPUSH mystack item" << endl;
    cout << "  SPOP mystack" << endl;
    cout << "  QPUSH myqueue item" << endl;
    cout << "  QPOP myqueue" << endl;
    cout << "  SADD myset value" << endl;
    cout << "  SREM myset value" << endl;
    cout << "  SISMEMBER myset value" << endl;
    cout << endl;
    cout << "Все операции выполняются за O(1)" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    string filename;
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--file" && i + 1 < argc) {
            filename = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
    }
    
    if (filename.empty()) {
        cerr << "Ошибка: Необходимо указать аргумент --file" << endl;
        printUsage();
        return 1;
    }
    
    try {
        Database db;
        initDatabase(db);
        
        cout << "Загрузка базы данных из файла: " << filename << endl;
        loadDatabase(db, filename);
        
        cout << endl << "База данных готова к работе." << endl;
        cout << "Вводите команды (для выхода введите 'exit'):" << endl;
        
        string query;
        while (true) {
            cout << "> ";
            getline(cin, query);
            
            if (query == "exit" || query == "quit" || query == "выход") {
                break;
            }
            
            if (query.empty()) {
                continue;
            }
            
            string result = executeQuery(db, query);
            
            if (!result.empty()) {
                cout << result << endl;
            }
            
            saveDatabase(db, filename);
        }
        
        freeDatabase(db);
        
        cout << "Работа завершена. Данные сохранены в файл: " << filename << endl;
        
    } catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
