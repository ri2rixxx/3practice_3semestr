#include "hash_table.h"
#include <iostream>
using namespace std;

const int INITIAL_CAPACITY = 16;

void initHashTable(HashTable& table, const string& table_name) {
    table.capacity = INITIAL_CAPACITY;
    table.size = 0;
    table.buckets = new HashEntry*[table.capacity];
    for (int i = 0; i < table.capacity; i++) {
        table.buckets[i] = nullptr;
    }
    table.name = table_name;
}

void freeHashTable(HashTable& table) {
    for (int i = 0; i < table.capacity; i++) {
        HashEntry* entry = table.buckets[i];
        while (entry != nullptr) {
            HashEntry* next = entry->next;
            delete entry;
            entry = next;
        }
    }
    delete[] table.buckets;
    table.buckets = nullptr;
    table.size = 0;
    table.capacity = 0;
    table.name = "";
}

int hashFunction(const string& key, int capacity) {
    unsigned long hash = 5381;
    for (char c : key) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % capacity;
}

bool hashSet(HashTable& table, const string& key, const string& value) {
    int index = hashFunction(key, table.capacity);
    HashEntry* entry = table.buckets[index];
    
    while (entry != nullptr) {
        if (entry->key == key) {
            entry->value = value;
            return true;
        }
        entry = entry->next;
    }
    
    HashEntry* newEntry = new HashEntry;
    newEntry->key = key;
    newEntry->value = value;
    newEntry->next = table.buckets[index];
    table.buckets[index] = newEntry;
    table.size++;
    
    return true;
}

string hashGet(const HashTable& table, const string& key) {
    int index = hashFunction(key, table.capacity);
    HashEntry* entry = table.buckets[index];
    
    while (entry != nullptr) {
        if (entry->key == key) {
            return entry->value;
        }
        entry = entry->next;
    }
    
    return "";
}

bool hashDelete(HashTable& table, const string& key) {
    int index = hashFunction(key, table.capacity);
    HashEntry* entry = table.buckets[index];
    HashEntry* prev = nullptr;
    
    while (entry != nullptr) {
        if (entry->key == key) {
            if (prev == nullptr) {
                table.buckets[index] = entry->next;
            } else {
                prev->next = entry->next;
            }
            delete entry;
            table.size--;
            return true;
        }
        prev = entry;
        entry = entry->next;
    }
    
    return false;
}
