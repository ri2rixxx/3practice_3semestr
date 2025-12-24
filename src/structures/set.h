#ifndef SET_H
#define SET_H

#include <string>
using namespace std;

struct SetNode {
    string value;
    SetNode* next;
};

struct Set {
    SetNode** buckets;
    int capacity;
    int size;
    string name;
};

void initSet(Set& set, const string& set_name);
void freeSet(Set& set);
bool setAdd(Set& set, const string& value);
bool setRemove(Set& set, const string& value);
bool setContains(const Set& set, const string& value);

#endif
