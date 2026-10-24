#ifndef QUEUE_H
#define QUEUE_H

#include <stdexcept>
#include "DoublyLinkedList.h"

template <typename T>
class Queue {
private:
    DoublyLinkedList<T> data;

public:
    Queue();
    bool empty() const;
    int size() const;
    void enqueue(const T& value);
    T dequeue();
    T& front();
    const T& front() const;
    void clear();
};

template <typename T>
Queue<T>::Queue() {
}

template <typename T>
bool Queue<T>::empty() const {
    return data.isEmpty();
}

template <typename T>
int Queue<T>::size() const {
    return data.size();
}

template <typename T>
void Queue<T>::enqueue(const T& value) {
    data.pushBack(value);
}

template <typename T>
T Queue<T>::dequeue() {
    if (empty()) {
        throw std::logic_error("Queue is empty.");
    }

    T value = data.at(0);
    data.removeFront();
    return value;
}

template <typename T>
T& Queue<T>::front() {
    if (empty()) {
        throw std::logic_error("Queue is empty.");
    }
    return data.at(0);
}

template <typename T>
const T& Queue<T>::front() const {
    if (empty()) {
        throw std::logic_error("Queue is empty.");
    }
    return data.at(0);
}

template <typename T>
void Queue<T>::clear() {
    data.clear();
}

#endif
