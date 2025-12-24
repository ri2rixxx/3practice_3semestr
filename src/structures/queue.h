#ifndef QUEUE_H
#define QUEUE_H

#include <string>
using namespace std;

struct Queue {
    string* data;
    int front;
    int rear;
    int capacity;
    int size;
    string name;
};

void initQueue(Queue& queue, const string& queue_name);
void freeQueue(Queue& queue);
void enqueue(Queue& queue, const string& value);
string dequeue(Queue& queue);
bool isEmptyQueue(const Queue& queue);

#endif
