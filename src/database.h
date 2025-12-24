#ifndef DATABASE_H
#define DATABASE_H

#include "structures/stack.h"
#include "structures/queue.h"
#include "structures/hash_table.h"
#include "structures/set.h"
#include <string>
#include <vector>
#include <mutex>

using namespace std;

struct Database {
    vector<Stack*> stacks;
    vector<Queue*> queues;
    vector<HashTable*> hashTables;
    vector<Set*> sets;
    
    // Мьютекс для потокобезопасности
    mutable mutex db_mutex;
};

void initDatabase(Database& db);
void freeDatabase(Database& db);
string executeQuery(Database& db, const string& query);
void saveDatabase(const Database& db, const string& filename);
void loadDatabase(Database& db, const string& filename);
vector<string> splitString(const string& str, char delimiter);
Stack* findStack(Database& db, const string& name);
Queue* findQueue(Database& db, const string& name);
HashTable* findHashTable(Database& db, const string& name);
Set* findSet(Database& db, const string& name);

#endif
