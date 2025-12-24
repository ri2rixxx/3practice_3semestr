#include "database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

void initDatabase(Database& db) {
}

void freeDatabase(Database& db) {
    lock_guard<mutex> lock(db.db_mutex);
    
    for (Stack* stack : db.stacks) {
        freeStack(*stack);
        delete stack;
    }
    db.stacks.clear();
    
    for (Queue* queue : db.queues) {
        freeQueue(*queue);
        delete queue;
    }
    db.queues.clear();
    
    for (HashTable* table : db.hashTables) {
        freeHashTable(*table);
        delete table;
    }
    db.hashTables.clear();
    
    for (Set* set : db.sets) {
        freeSet(*set);
        delete set;
    }
    db.sets.clear();
}

vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

Stack* findStack(Database& db, const string& name) {
    lock_guard<mutex> lock(db.db_mutex);
    for (Stack* stack : db.stacks) {
        if (stack->name == name) {
            return stack;
        }
    }
    return nullptr;
}

Queue* findQueue(Database& db, const string& name) {
    lock_guard<mutex> lock(db.db_mutex);
    for (Queue* queue : db.queues) {
        if (queue->name == name) {
            return queue;
        }
    }
    return nullptr;
}

HashTable* findHashTable(Database& db, const string& name) {
    lock_guard<mutex> lock(db.db_mutex);
    for (HashTable* table : db.hashTables) {
        if (table->name == name) {
            return table;
        }
    }
    return nullptr;
}

Set* findSet(Database& db, const string& name) {
    lock_guard<mutex> lock(db.db_mutex);
    for (Set* set : db.sets) {
        if (set->name == name) {
            return set;
        }
    }
    return nullptr;
}

string executeQuery(Database& db, const string& query) {
    lock_guard<mutex> lock(db.db_mutex);

    vector<string> tokens;
    stringstream ss(query);
    string token;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    if (tokens.empty()) {
        return "ОШИБКА: Пустой запрос";
    }
    
    string command = tokens[0];
    
    // HSET таблица ключ значение
    if (command == "HSET") {
        if (tokens.size() < 4) {
            return "ОШИБКА: HSET требует имя_таблицы ключ значение";
        }
        
        string hashname = tokens[1];
        string key = tokens[2];
        string value = tokens[3];
        
        // Ищем хеш-таблицу
        HashTable* table = nullptr;
        for (HashTable* ht : db.hashTables) {
            if (ht->name == hashname) {
                table = ht;
                break;
            }
        }
        
        // Создаем если не существует
        if (!table) {
            table = new HashTable;
            initHashTable(*table, hashname);
            db.hashTables.push_back(table);
        }
        
        // Устанавливаем значение
        hashSet(*table, key, value);

        return value;
    }
    
    // HGET таблица ключ
    else if (command == "HGET") {
        if (tokens.size() < 3) {
            return "ОШИБКА: HGET требует имя_таблицы ключ";
        }
        
        string hashname = tokens[1];
        string key = tokens[2];
        
        // Ищем хеш-таблицу
        HashTable* table = nullptr;
        for (HashTable* ht : db.hashTables) {
            if (ht->name == hashname) {
                table = ht;
                break;
            }
        }
        
        if (!table) {
            return "НЕ_НАЙДЕНО";
        }
        
        // Получаем значение
        string result = hashGet(*table, key);
        return result.empty() ? "НЕ_НАЙДЕНО" : result;
    }
    
    // HDEL таблица ключ
    else if (command == "HDEL") {
        if (tokens.size() < 3) {
            return "ОШИБКА: HDEL требует имя_таблицы ключ";
        }
        
        string hashname = tokens[1];
        string key = tokens[2];
        
        HashTable* table = nullptr;
        for (HashTable* ht : db.hashTables) {
            if (ht->name == hashname) {
                table = ht;
                break;
            }
        }
        
        if (!table) {
            return "НЕ_НАЙДЕНО";
        }
        
        bool deleted = hashDelete(*table, key);
        return deleted ? "УДАЛЕНО" : "НЕ_НАЙДЕНО";
    }
    
    // SPUSH стек значение
    else if (command == "SPUSH") {
        if (tokens.size() < 3) {
            return "ОШИБКА: SPUSH требует имя_стека значение";
        }
        
        string stackname = tokens[1];
        string value = tokens[2];
        
        Stack* stack = nullptr;
        for (Stack* s : db.stacks) {
            if (s->name == stackname) {
                stack = s;
                break;
            }
        }
        
        if (!stack) {
            stack = new Stack;
            initStack(*stack, stackname);
            db.stacks.push_back(stack);
        }
        
        pushStack(*stack, value);
        return value;
    }
    
    // SPOP стек
    else if (command == "SPOP") {
        if (tokens.size() < 2) {
            return "ОШИБКА: SPOP требует имя_стека";
        }
        
        string stackname = tokens[1];
        
        Stack* stack = nullptr;
        for (Stack* s : db.stacks) {
            if (s->name == stackname) {
                stack = s;
                break;
            }
        }
        
        if (!stack) {
            return "ПУСТО";
        }
        
        if (isEmptyStack(*stack)) {
            return "ПУСТО";
        }
        
        string result = popStack(*stack);
        return result;
    }
    
    // QPUSH очередь значение
    else if (command == "QPUSH") {
        if (tokens.size() < 3) {
            return "ОШИБКА: QPUSH требует имя_очереди значение";
        }
        
        string queuename = tokens[1];
        string value = tokens[2];
        
        Queue* queue = nullptr;
        for (Queue* q : db.queues) {
            if (q->name == queuename) {
                queue = q;
                break;
            }
        }
        
        if (!queue) {
            queue = new Queue;
            initQueue(*queue, queuename);
            db.queues.push_back(queue);
        }
        
        enqueue(*queue, value);
        return value;
    }
    
    // QPOP очередь
    else if (command == "QPOP") {
        if (tokens.size() < 2) {
            return "ОШИБКА: QPOP требует имя_очереди";
        }
        
        string queuename = tokens[1];
        
        Queue* queue = nullptr;
        for (Queue* q : db.queues) {
            if (q->name == queuename) {
                queue = q;
                break;
            }
        }
        
        if (!queue) {
            return "ПУСТО";
        }
        
        if (isEmptyQueue(*queue)) {
            return "ПУСТО";
        }
        
        string result = dequeue(*queue);
        return result;
    }
    
    // SADD множество значение
    else if (command == "SADD") {
        if (tokens.size() < 3) {
            return "ОШИБКА: SADD требует имя_множества значение";
        }
        
        string setname = tokens[1];
        string value = tokens[2];
        
        Set* set = nullptr;
        for (Set* s : db.sets) {
            if (s->name == setname) {
                set = s;
                break;
            }
        }
        
        if (!set) {
            set = new Set;
            initSet(*set, setname);
            db.sets.push_back(set);
        }
        
        bool added = setAdd(*set, value);
        return added ? "ДОБАВЛЕНО" : "УЖЕ_СУЩЕСТВУЕТ";
    }
    
    // SREM множество значение
    else if (command == "SREM") {
        if (tokens.size() < 3) {
            return "ОШИБКА: SREM требует имя_множества значение";
        }
        
        string setname = tokens[1];
        string value = tokens[2];
        
        Set* set = nullptr;
        for (Set* s : db.sets) {
            if (s->name == setname) {
                set = s;
                break;
            }
        }
        
        if (!set) {
            return "НЕ_НАЙДЕНО";
        }
        
        bool removed = setRemove(*set, value);
        return removed ? "УДАЛЕНО" : "НЕ_НАЙДЕНО";
    }
    
    // SISMEMBER множество значение
    else if (command == "SISMEMBER") {
        if (tokens.size() < 3) {
            return "ОШИБКА: SISMEMBER требует имя_множества значение";
        }
        
        string setname = tokens[1];
        string value = tokens[2];
        
        Set* set = nullptr;
        for (Set* s : db.sets) {
            if (s->name == setname) {
                set = s;
                break;
            }
        }
        
        if (!set) {
            return "ЛОЖЬ";
        }
        
        bool exists = setContains(*set, value);
        return exists ? "ИСТИНА" : "ЛОЖЬ";
    }
    
    return "ОШИБКА: Неизвестная команда";
}

void saveDatabase(const Database& db, const string& filename) {
    lock_guard<mutex> lock(db.db_mutex);
    
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file for writing: " + filename);
    }
    
    int stackCount = db.stacks.size();
    file.write(reinterpret_cast<const char*>(&stackCount), sizeof(stackCount));
    
    for (const Stack* stack : db.stacks) {
        int nameLength = stack->name.length();
        file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        file.write(stack->name.c_str(), nameLength);
        
        file.write(reinterpret_cast<const char*>(&stack->size), sizeof(stack->size));
        for (int i = 0; i < stack->size; i++) {
            int valueLength = stack->data[i].length();
            file.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
            file.write(stack->data[i].c_str(), valueLength);
        }
    }
    
    int queueCount = db.queues.size();
    file.write(reinterpret_cast<const char*>(&queueCount), sizeof(queueCount));
    
    for (const Queue* queue : db.queues) {
        int nameLength = queue->name.length();
        file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        file.write(queue->name.c_str(), nameLength);
        
        file.write(reinterpret_cast<const char*>(&queue->size), sizeof(queue->size));
        for (int i = 0; i < queue->size; i++) {
            int index = (queue->front + i) % queue->capacity;
            int valueLength = queue->data[index].length();
            file.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
            file.write(queue->data[index].c_str(), valueLength);
        }
    }
    
    int hashTableCount = db.hashTables.size();
    file.write(reinterpret_cast<const char*>(&hashTableCount), sizeof(hashTableCount));
    
    for (const HashTable* table : db.hashTables) {
        int nameLength = table->name.length();
        file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        file.write(table->name.c_str(), nameLength);
        
        file.write(reinterpret_cast<const char*>(&table->size), sizeof(table->size));
        
        for (int i = 0; i < table->capacity; i++) {
            HashEntry* entry = table->buckets[i];
            while (entry != nullptr) {
                int keyLength = entry->key.length();
                file.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
                file.write(entry->key.c_str(), keyLength);
                
                int valueLength = entry->value.length();
                file.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
                file.write(entry->value.c_str(), valueLength);
                
                entry = entry->next;
            }
        }
    }
    
    int setCount = db.sets.size();
    file.write(reinterpret_cast<const char*>(&setCount), sizeof(setCount));
    
    for (const Set* set : db.sets) {
        int nameLength = set->name.length();
        file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        file.write(set->name.c_str(), nameLength);
        
        file.write(reinterpret_cast<const char*>(&set->size), sizeof(set->size));
        
        for (int i = 0; i < set->capacity; i++) {
            SetNode* node = set->buckets[i];
            while (node != nullptr) {
                int valueLength = node->value.length();
                file.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
                file.write(node->value.c_str(), valueLength);
                node = node->next;
            }
        }
    }
    
    file.close();
}

void loadDatabase(Database& db, const string& filename) {
    lock_guard<mutex> lock(db.db_mutex);
    
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        return;
    }
    
    int stackCount;
    file.read(reinterpret_cast<char*>(&stackCount), sizeof(stackCount));
    
    for (int i = 0; i < stackCount; i++) {
        Stack* stack = new Stack;
        
        int nameLength;
        file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        char* nameBuffer = new char[nameLength + 1];
        file.read(nameBuffer, nameLength);
        nameBuffer[nameLength] = '\0';
        initStack(*stack, string(nameBuffer));
        delete[] nameBuffer;
        
        int size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        
        for (int j = 0; j < size; j++) {
            int valueLength;
            file.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
            char* valueBuffer = new char[valueLength + 1];
            file.read(valueBuffer, valueLength);
            valueBuffer[valueLength] = '\0';
            pushStack(*stack, string(valueBuffer));
            delete[] valueBuffer;
        }
        
        db.stacks.push_back(stack);
    }
    
    int queueCount;
    file.read(reinterpret_cast<char*>(&queueCount), sizeof(queueCount));
    
    for (int i = 0; i < queueCount; i++) {
        Queue* queue = new Queue;
        
        int nameLength;
        file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        char* nameBuffer = new char[nameLength + 1];
        file.read(nameBuffer, nameLength);
        nameBuffer[nameLength] = '\0';
        initQueue(*queue, string(nameBuffer));
        delete[] nameBuffer;
        
        int size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        
        for (int j = 0; j < size; j++) {
            int valueLength;
            file.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
            char* valueBuffer = new char[valueLength + 1];
            file.read(valueBuffer, valueLength);
            valueBuffer[valueLength] = '\0';
            enqueue(*queue, string(valueBuffer));
            delete[] valueBuffer;
        }
        
        db.queues.push_back(queue);
    }
    
    int hashTableCount;
    file.read(reinterpret_cast<char*>(&hashTableCount), sizeof(hashTableCount));
    
    for (int i = 0; i < hashTableCount; i++) {
        HashTable* table = new HashTable;
        
        int nameLength;
        file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        char* nameBuffer = new char[nameLength + 1];
        file.read(nameBuffer, nameLength);
        nameBuffer[nameLength] = '\0';
        initHashTable(*table, string(nameBuffer));
        delete[] nameBuffer;
        
        int size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        
        for (int j = 0; j < size; j++) {
            int keyLength;
            file.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
            char* keyBuffer = new char[keyLength + 1];
            file.read(keyBuffer, keyLength);
            keyBuffer[keyLength] = '\0';
            string key = string(keyBuffer);
            delete[] keyBuffer;
            
            int valueLength;
            file.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
            char* valueBuffer = new char[valueLength + 1];
            file.read(valueBuffer, valueLength);
            valueBuffer[valueLength] = '\0';
            string value = string(valueBuffer);
            delete[] valueBuffer;
            
            hashSet(*table, key, value);
        }
        
        db.hashTables.push_back(table);
    }

    int setCount;
    file.read(reinterpret_cast<char*>(&setCount), sizeof(setCount));
    
    for (int i = 0; i < setCount; i++) {
        Set* set = new Set;
        
        int nameLength;
        file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        char* nameBuffer = new char[nameLength + 1];
        file.read(nameBuffer, nameLength);
        nameBuffer[nameLength] = '\0';
        initSet(*set, string(nameBuffer));
        delete[] nameBuffer;
        
        int size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        
        for (int j = 0; j < size; j++) {
            int valueLength;
            file.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
            char* valueBuffer = new char[valueLength + 1];
            file.read(valueBuffer, valueLength);
            valueBuffer[valueLength] = '\0';
            setAdd(*set, string(valueBuffer));
            delete[] valueBuffer;
        }
        
        db.sets.push_back(set);
    }
    
    file.close();
}
