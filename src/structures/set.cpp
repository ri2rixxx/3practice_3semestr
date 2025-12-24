#include "set.h"
#include <iostream>
using namespace std;

const int SET_INITIAL_CAPACITY = 16;

void initSet(Set& set, const string& set_name) {
    set.capacity = SET_INITIAL_CAPACITY;
    set.size = 0;
    set.buckets = new SetNode*[set.capacity];
    for (int i = 0; i < set.capacity; i++) {
        set.buckets[i] = nullptr;
    }
    set.name = set_name;
}

void freeSet(Set& set) {
    for (int i = 0; i < set.capacity; i++) {
        SetNode* node = set.buckets[i];
        while (node != nullptr) {
            SetNode* next = node->next;
            delete node;
            node = next;
        }
    }
    delete[] set.buckets;
    set.buckets = nullptr;
    set.size = 0;
    set.capacity = 0;
    set.name = "";
}

int setHashFunction(const string& value, int capacity) {
    unsigned long hash = 5381;
    for (char c : value) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % capacity;
}

bool setAdd(Set& set, const string& value) {
    int index = setHashFunction(value, set.capacity);
    SetNode* node = set.buckets[index];
    
    while (node != nullptr) {
        if (node->value == value) {
            return false;
        }
        node = node->next;
    }
    
    SetNode* newNode = new SetNode;
    newNode->value = value;
    newNode->next = set.buckets[index];
    set.buckets[index] = newNode;
    set.size++;
    
    return true;
}

bool setRemove(Set& set, const string& value) {
    int index = setHashFunction(value, set.capacity);
    SetNode* node = set.buckets[index];
    SetNode* prev = nullptr;
    
    while (node != nullptr) {
        if (node->value == value) {
            if (prev == nullptr) {
                set.buckets[index] = node->next;
            } else {
                prev->next = node->next;
            }
            delete node;
            set.size--;
            return true;
        }
        prev = node;
        node = node->next;
    }
    
    return false;
}

bool setContains(const Set& set, const string& value) {
    int index = setHashFunction(value, set.capacity);
    SetNode* node = set.buckets[index];
    
    while (node != nullptr) {
        if (node->value == value) {
            return true;
        }
        node = node->next;
    }
    
    return false;
}
