// ============================================================================
//  Queue.h - Hang doi FIFO xay tren DoublyLinkedList (template)
//  enqueue = addLast (them cuoi)   |   dequeue = removeFirst (lay dau)
//  Cai dat: Queue.tpp
// ============================================================================
#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include "DoublyLinkedList.h"

template <typename T>
class Queue
{
private:
    DoublyLinkedList<T> list;

public:
    void enqueue(const T& value);
    bool dequeue(T& out);              // lay phan tu dau ra 'out'; false neu rong
    T* front() const;                  // xem phan tu dau; nullptr neu rong
    bool isEmpty() const;
    int size() const;
    void clear();
    void display() const;              // T phai co operator<<
};

#include "Queue.tpp"

#endif
