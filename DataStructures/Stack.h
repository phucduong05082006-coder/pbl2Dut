#ifndef STACK_H
#define STACK_H

#include <stdexcept>
#include "DoublyLinkedList.h"

template <typename T>
class Stack {
private:
    DoublyLinkedList<T> data;

public:
    Stack();
    bool empty() const;
    int size() const;
    void push(const T& value);
    T pop();
    T& top();
    const T& top() const;
    void clear();
};

template <typename T>
Stack<T>::Stack() {
}

template <typename T>
bool Stack<T>::empty() const {
    return data.isEmpty();
}

template <typename T>
int Stack<T>::size() const {
    return data.size();
}

template <typename T>
void Stack<T>::push(const T& value) {
    data.pushFront(value);
}

template <typename T>
T Stack<T>::pop() {
    if (empty()) {
        throw std::logic_error("Stack is empty.");
    }

    T value = data.at(0);
    data.removeFront();
    return value;
}

template <typename T>
T& Stack<T>::top() {
    if (empty()) {
        throw std::logic_error("Stack is empty.");
    }
    return data.at(0);
}

template <typename T>
const T& Stack<T>::top() const {
    if (empty()) {
        throw std::logic_error("Stack is empty.");
    }
    return data.at(0);
}

template <typename T>
void Stack<T>::clear() {
    data.clear();
}

#endif
