#ifndef STACK_H
#define STACK_H

#include <string>
using namespace std;

struct Stack {
    string* data;
    int size;
    int capacity;
    string name;
};

void initStack(Stack& stack, const string& stack_name);
void freeStack(Stack& stack);
void pushStack(Stack& stack, const string& value);
string popStack(Stack& stack);
bool isEmptyStack(const Stack& stack);

#endif
