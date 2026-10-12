// ============================================================================
//  BST.h - Cay nhi phan tim kiem (template)
//  - Hai phan tu duoc so sanh bang ham CompareFunc (comparator) do nguoi dung truyen vao:
//    tra ve <0 neu a<b, 0 neu bang nhau, >0 neu a>b.
//  - Khong chap nhan hai phan tu "bang nhau" (compare == 0).
//  Cai dat: BST.tpp
// ============================================================================
#ifndef BST_H
#define BST_H

#include <iostream>
#include <string>

template <typename T>
struct BSTNode
{
    T data;
    BSTNode<T>* left;
    BSTNode<T>* right;

    explicit BSTNode(const T& value);
};

template <typename T>
class BST
{
public:
    typedef int (*CompareFunc)(const T&, const T&);
    typedef void (*VisitFunc)(const T&);
    typedef std::string (*LabelFunc)(const T&);

private:
    BSTNode<T>* root;
    int count;
    CompareFunc compare;

    BSTNode<T>* insertNode(BSTNode<T>* node, const T& value, bool& inserted);
    BSTNode<T>* removeNode(BSTNode<T>* node, const T& key, bool& removed);
    void destroy(BSTNode<T>* node);                                    // xoa theo postorder
    void inorderRec(BSTNode<T>* node, VisitFunc visit) const;
    void preorderRec(BSTNode<T>* node, VisitFunc visit) const;
    void postorderRec(BSTNode<T>* node, VisitFunc visit) const;
    int heightRec(BSTNode<T>* node) const;
    void printRec(BSTNode<T>* node, int depth, LabelFunc label) const;

public:
    explicit BST(CompareFunc cmp);
    ~BST();

    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    bool insert(const T& value);                                       // O(h)
    // Tra ve con tro toi du lieu trong cay (nullptr neu khong co).
    // 'steps' (tuy chon) nhan so lan so sanh da thuc hien.
    T* search(const T& key, int* steps = nullptr) const;               // O(h)
    bool remove(const T& key);                                         // O(h)

    void inorder(VisitFunc visit) const;                               // tang dan
    void preorder(VisitFunc visit) const;
    void postorder(VisitFunc visit) const;
    void printTree(LabelFunc label) const;                             // ve cay xoay ngang

    int size() const;
    int height() const;
    bool isEmpty() const;
    void clear();
};

#include "BST.tpp"

#endif
