#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
using namespace std;

struct HashEntry {
    string key;
    string value;
    HashEntry* next;
};

struct HashTable {
    HashEntry** buckets;
    int capacity;
    int size;
    string name;
};

void initHashTable(HashTable& table, const string& table_name);
void freeHashTable(HashTable& table);
bool hashSet(HashTable& table, const string& key, const string& value);
string hashGet(const HashTable& table, const string& key);
bool hashDelete(HashTable& table, const string& key);
int hashFunction(const string& key, int capacity);

#endif
