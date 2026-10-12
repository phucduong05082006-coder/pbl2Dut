// ============================================================================
//  Node.h - Node cua danh sach lien ket doi (template)
//  Phan cai dat nam trong Node.tpp (template phai duoc compiler nhin thay).
// ============================================================================
#ifndef NODE_H
#define NODE_H

template <typename T>
struct Node
{
    T data;
    Node<T>* prev;
    Node<T>* next;

    explicit Node(const T& value);
};

#include "Node.tpp"

#endif
