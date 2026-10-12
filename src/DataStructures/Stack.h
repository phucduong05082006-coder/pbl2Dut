// ============================================================================
//  Stack.h - Ngan xep LIFO xay tren DoublyLinkedList (template)
//  push = addLast   |   pop = removeLast   |   top = tail
//  Cai dat: Stack.tpp
// ============================================================================
#ifndef STACK_H
#define STACK_H

#include <iostream>
#include "DoublyLinkedList.h"

template <typename T>
class Stack
{
private:
    DoublyLinkedList<T> list;

public:
    void push(const T& value);
    bool pop(T& out);                  // lay phan tu tren cung ra 'out'; false neu rong
    T* top() const;                    // xem phan tu tren cung; nullptr neu rong
    bool isEmpty() const;
    int size() const;
    void clear();
    void display() const;              // in tu TOP xuong BOTTOM (T phai co operator<<)
};

#include "Stack.tpp"

#endif
