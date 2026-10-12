// ============================================================================
//  DoublyLinkedList.h - Danh sach lien ket doi (template)
//  Khai bao class. Cai dat: DoublyLinkedList.tpp (duoc include o cuoi file nay).
// ============================================================================
#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include <iostream>
#include "Node.h"

template <typename T>
class DoublyLinkedList
{
private:
    Node<T>* head;
    Node<T>* tail;
    int count;

    Node<T>* nodeAt(int index) const;              // tim node theo vi tri (di tu dau hoac cuoi)
    void unlink(Node<T>* node);                    // go 1 node va giai phong bo nho
    void copyFrom(const DoublyLinkedList<T>& other);

public:
    DoublyLinkedList();
    DoublyLinkedList(const DoublyLinkedList<T>& other);              // Rule of Three: deep copy
    DoublyLinkedList<T>& operator=(const DoublyLinkedList<T>& other);
    ~DoublyLinkedList();

    // ---- Them ----
    void addFirst(const T& value);                 // O(1)
    void addLast(const T& value);                  // O(1)

    // ---- Xoa ----
    bool removeFirst();                            // O(1)
    bool removeLast();                             // O(1)
    bool removeAt(int index);                      // O(n)
    void clear();

    // ---- Truy cap / tim kiem ----
    T* getAt(int index) const;                     // O(n) - nullptr neu index sai
    Node<T>* findNode(const T& value) const;       // O(n) - dung operator== cua T
    T* find(const T& value) const;                 // O(n)
    int indexOf(const T& value) const;             // O(n) - -1 neu khong co
    bool contains(const T& value) const;

    bool isEmpty() const;
    int size() const;
    Node<T>* getHead() const;
    Node<T>* getTail() const;

    // ---- Duyet ----
    void forEach(void (*action)(const T&)) const;          // dau -> cuoi
    void forEachReverse(void (*action)(const T&)) const;   // cuoi -> dau
    void display(const char* separator = " <-> ") const;         // T phai co operator<<
    void displayReverse(const char* separator = " <-> ") const;
};

#include "DoublyLinkedList.tpp"

#endif
