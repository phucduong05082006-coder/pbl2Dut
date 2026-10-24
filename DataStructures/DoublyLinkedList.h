#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <stdexcept>

template <typename T>
class DoublyLinkedList {
private:
    struct Node {
        T value;
        Node* prev;
        Node* next;

        explicit Node(const T& itemValue);
    };

    Node* head;
    Node* tail;
    int count;

    Node* getNodeAt(int index) const;
    void copyFrom(const DoublyLinkedList& other);
    void swap(DoublyLinkedList& other);

public:
    DoublyLinkedList();
    ~DoublyLinkedList();
    DoublyLinkedList(const DoublyLinkedList& other);
    DoublyLinkedList& operator=(const DoublyLinkedList& other);

    bool isEmpty() const;
    int size() const;
    void pushFront(const T& value);
    void pushBack(const T& value);
    void insertAt(int index, const T& value);
    void removeFront();
    void removeBack();
    void removeAt(int index);
    int findIndex(const T& value) const;
    bool findIf(bool (*predicate)(const T& value)) const;
    T& at(int index);
    const T& at(int index) const;
    void traverseForward(void (*callback)(const T& value)) const;
    void traverseBackward(void (*callback)(const T& value)) const;
    void clear();
};

template <typename T>
DoublyLinkedList<T>::Node::Node(const T& itemValue)
    : value(itemValue), prev(NULL), next(NULL) {
}

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList()
    : head(NULL), tail(NULL), count(0) {
}

template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList() {
    clear();
}

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList(const DoublyLinkedList& other)
    : head(NULL), tail(NULL), count(0) {
    try {
        copyFrom(other);
    }
    catch (...) {
        clear();
        throw;
    }
}

template <typename T>
DoublyLinkedList<T>& DoublyLinkedList<T>::operator=(const DoublyLinkedList& other) {
    if (this != &other) {
        DoublyLinkedList copy(other);
        swap(copy);
    }
    return *this;
}

template <typename T>
void DoublyLinkedList<T>::swap(DoublyLinkedList& other) {
    Node* temporaryHead = head;
    head = other.head;
    other.head = temporaryHead;

    Node* temporaryTail = tail;
    tail = other.tail;
    other.tail = temporaryTail;

    int temporaryCount = count;
    count = other.count;
    other.count = temporaryCount;
}

template <typename T>
typename DoublyLinkedList<T>::Node* DoublyLinkedList<T>::getNodeAt(int index) const {
    if (index < 0 || index >= count) {
        throw std::out_of_range("Index is out of range.");
    }

    Node* current = head;
    for (int i = 0; i < index; ++i) {
        current = current->next;
    }

    return current;
}

template <typename T>
void DoublyLinkedList<T>::copyFrom(const DoublyLinkedList& other) {
    Node* current = other.head;
    while (current != NULL) {
        pushBack(current->value);
        current = current->next;
    }
}

template <typename T>
bool DoublyLinkedList<T>::isEmpty() const {
    return count == 0;
}

template <typename T>
int DoublyLinkedList<T>::size() const {
    return count;
}

template <typename T>
void DoublyLinkedList<T>::pushFront(const T& value) {
    Node* newNode = new Node(value);

    if (isEmpty()) {
        head = newNode;
        tail = newNode;
    }
    else {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }

    ++count;
}

template <typename T>
void DoublyLinkedList<T>::pushBack(const T& value) {
    Node* newNode = new Node(value);

    if (isEmpty()) {
        head = newNode;
        tail = newNode;
    }
    else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }

    ++count;
}

template <typename T>
void DoublyLinkedList<T>::insertAt(int index, const T& value) {
    if (index < 0 || index > count) {
        throw std::out_of_range("Insert position is invalid.");
    }

    if (index == 0) {
        pushFront(value);
        return;
    }

    if (index == count) {
        pushBack(value);
        return;
    }

    Node* current = getNodeAt(index);
    Node* newNode = new Node(value);
    newNode->prev = current->prev;
    newNode->next = current;
    current->prev->next = newNode;
    current->prev = newNode;
    ++count;
}

template <typename T>
void DoublyLinkedList<T>::removeFront() {
    if (isEmpty()) {
        throw std::logic_error("Cannot remove from an empty list.");
    }

    Node* removed = head;
    if (count == 1) {
        head = NULL;
        tail = NULL;
    }
    else {
        head = head->next;
        head->prev = NULL;
    }

    delete removed;
    --count;
}

template <typename T>
void DoublyLinkedList<T>::removeBack() {
    if (isEmpty()) {
        throw std::logic_error("Cannot remove from an empty list.");
    }

    Node* removed = tail;
    if (count == 1) {
        head = NULL;
        tail = NULL;
    }
    else {
        tail = tail->prev;
        tail->next = NULL;
    }

    delete removed;
    --count;
}

template <typename T>
void DoublyLinkedList<T>::removeAt(int index) {
    if (isEmpty()) {
        throw std::logic_error("Cannot remove from an empty list.");
    }
    if (index < 0 || index >= count) {
        throw std::out_of_range("Remove position is invalid.");
    }
    if (index == 0) {
        removeFront();
        return;
    }
    if (index == count - 1) {
        removeBack();
        return;
    }

    Node* current = getNodeAt(index);
    current->prev->next = current->next;
    current->next->prev = current->prev;
    delete current;
    --count;
}

template <typename T>
int DoublyLinkedList<T>::findIndex(const T& value) const {
    Node* current = head;
    int index = 0;

    while (current != NULL) {
        if (current->value == value) {
            return index;
        }
        current = current->next;
        ++index;
    }

    return -1;
}

template <typename T>
bool DoublyLinkedList<T>::findIf(bool (*predicate)(const T& value)) const {
    Node* current = head;
    while (current != NULL) {
        if (predicate(current->value)) {
            return true;
        }
        current = current->next;
    }

    return false;
}

template <typename T>
T& DoublyLinkedList<T>::at(int index) {
    return getNodeAt(index)->value;
}

template <typename T>
const T& DoublyLinkedList<T>::at(int index) const {
    return getNodeAt(index)->value;
}

template <typename T>
void DoublyLinkedList<T>::traverseForward(void (*callback)(const T& value)) const {
    Node* current = head;
    while (current != NULL) {
        callback(current->value);
        current = current->next;
    }
}

template <typename T>
void DoublyLinkedList<T>::traverseBackward(void (*callback)(const T& value)) const {
    Node* current = tail;
    while (current != NULL) {
        callback(current->value);
        current = current->prev;
    }
}

template <typename T>
void DoublyLinkedList<T>::clear() {
    Node* current = head;
    while (current != NULL) {
        Node* nextNode = current->next;
        delete current;
        current = nextNode;
    }

    head = NULL;
    tail = NULL;
    count = 0;
}

#endif
