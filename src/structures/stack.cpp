#include "stack.h"
#include <iostream>
using namespace std;

void initStack(Stack& stack, const string& stack_name) {
    stack.capacity = 10;
    stack.size = 0;
    stack.data = new string[stack.capacity];
    stack.name = stack_name;
}

void freeStack(Stack& stack) {
    delete[] stack.data;
    stack.data = nullptr;
    stack.size = 0;
    stack.capacity = 0;
    stack.name = "";
}

void resizeStack(Stack& stack) {
    stack.capacity *= 2;
    string* newData = new string[stack.capacity];
    for (int i = 0; i < stack.size; i++) {
        newData[i] = stack.data[i];
    }
    delete[] stack.data;
    stack.data = newData;
}

void pushStack(Stack& stack, const string& value) {
    if (stack.size == stack.capacity) {
        resizeStack(stack);
    }
    stack.data[stack.size] = value;
    stack.size++;
}

string popStack(Stack& stack) {
    if (stack.size > 0) {
        string value = stack.data[stack.size - 1];
        stack.size--;
        return value;
    }
    return "";
}

bool isEmptyStack(const Stack& stack) {
    return stack.size == 0;
}
