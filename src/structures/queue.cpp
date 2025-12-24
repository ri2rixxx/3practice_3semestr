#include "queue.h"
#include <iostream>
using namespace std;

void initQueue(Queue& queue, const string& queue_name) {
    queue.capacity = 10;
    queue.size = 0;
    queue.front = 0;
    queue.rear = -1;
    queue.data = new string[queue.capacity];
    queue.name = queue_name;
}

void freeQueue(Queue& queue) {
    delete[] queue.data;
    queue.data = nullptr;
    queue.size = 0;
    queue.capacity = 0;
    queue.name = "";
}

void resizeQueue(Queue& queue) {
    int newCapacity = queue.capacity * 2;
    string* newData = new string[newCapacity];
    
    for (int i = 0; i < queue.size; i++) {
        newData[i] = queue.data[(queue.front + i) % queue.capacity];
    }
    
    delete[] queue.data;
    queue.data = newData;
    queue.front = 0;
    queue.rear = queue.size - 1;
    queue.capacity = newCapacity;
}

void enqueue(Queue& queue, const string& value) {
    if (queue.size == queue.capacity) {
        resizeQueue(queue);
    }
    
    queue.rear = (queue.rear + 1) % queue.capacity;
    queue.data[queue.rear] = value;
    queue.size++;
}

string dequeue(Queue& queue) {
    if (queue.size == 0) {
        return "";
    }
    
    string value = queue.data[queue.front];
    queue.front = (queue.front + 1) % queue.capacity;
    queue.size--;
    return value;
}

bool isEmptyQueue(const Queue& queue) {
    return queue.size == 0;
}
