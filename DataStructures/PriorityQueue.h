#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdexcept>
#include "Queue.h"

template <typename T>
class PriorityQueue {
private:
    struct HeapNode {
        T value;
        HeapNode* left;
        HeapNode* right;
        HeapNode* parent;

        explicit HeapNode(const T& itemValue);
    };

    HeapNode* root;
    int count;
    bool (*compareFunction)(const T&, const T&);

    static bool defaultCompare(const T& leftValue, const T& rightValue);
    HeapNode* findLastNode() const;
    HeapNode* findParentOf(HeapNode* targetNode) const;
    void swapValues(HeapNode* first, HeapNode* second);
    void bubbleUp(HeapNode* currentNode);
    void heapifyDown(HeapNode* currentNode);

public:
    explicit PriorityQueue(bool (*comparisonFunction)(const T&, const T&) = defaultCompare);
    ~PriorityQueue();
    PriorityQueue(const PriorityQueue&) = delete;
    PriorityQueue& operator=(const PriorityQueue&) = delete;

    bool empty() const;
    int size() const;
    void push(const T& value);
    T top() const;
    T pop();
    void clear();
};

template <typename T>
PriorityQueue<T>::HeapNode::HeapNode(const T& itemValue)
    : value(itemValue), left(NULL), right(NULL), parent(NULL) {
}

template <typename T>
bool PriorityQueue<T>::defaultCompare(const T& leftValue, const T& rightValue) {
    return leftValue > rightValue;
}

template <typename T>
PriorityQueue<T>::PriorityQueue(bool (*comparisonFunction)(const T&, const T&))
    : root(NULL), count(0), compareFunction(comparisonFunction) {
}

template <typename T>
PriorityQueue<T>::~PriorityQueue() {
    clear();
}

template <typename T>
typename PriorityQueue<T>::HeapNode* PriorityQueue<T>::findLastNode() const {
    if (root == NULL) {
        return NULL;
    }

    Queue<HeapNode*> queue;
    queue.enqueue(root);
    HeapNode* lastNode = root;
    while (!queue.empty()) {
        HeapNode* current = queue.dequeue();
        if (current->left != NULL) {
            lastNode = current->left;
            queue.enqueue(current->left);
        }
        if (current->right != NULL) {
            lastNode = current->right;
            queue.enqueue(current->right);
        }
    }
    return lastNode;
}

template <typename T>
typename PriorityQueue<T>::HeapNode* PriorityQueue<T>::findParentOf(HeapNode* targetNode) const {
    if (targetNode == NULL || root == NULL || targetNode == root) {
        return NULL;
    }

    Queue<HeapNode*> queue;
    queue.enqueue(root);
    while (!queue.empty()) {
        HeapNode* current = queue.dequeue();
        if (current->left == targetNode || current->right == targetNode) {
            return current;
        }
        if (current->left != NULL) {
            queue.enqueue(current->left);
        }
        if (current->right != NULL) {
            queue.enqueue(current->right);
        }
    }
    return NULL;
}

template <typename T>
void PriorityQueue<T>::swapValues(HeapNode* first, HeapNode* second) {
    T temporary = first->value;
    first->value = second->value;
    second->value = temporary;
}

template <typename T>
void PriorityQueue<T>::bubbleUp(HeapNode* currentNode) {
    while (currentNode != NULL &&
           currentNode->parent != NULL &&
           compareFunction(currentNode->value, currentNode->parent->value)) {
        swapValues(currentNode, currentNode->parent);
        currentNode = currentNode->parent;
    }
}

template <typename T>
void PriorityQueue<T>::heapifyDown(HeapNode* currentNode) {
    while (currentNode != NULL) {
        HeapNode* higherPriorityChild = currentNode;

        if (currentNode->left != NULL &&
            compareFunction(currentNode->left->value, currentNode->value)) {
            higherPriorityChild = currentNode->left;
        }
        if (currentNode->right != NULL &&
            compareFunction(currentNode->right->value, higherPriorityChild->value)) {
            higherPriorityChild = currentNode->right;
        }
        if (higherPriorityChild == currentNode) {
            break;
        }

        swapValues(currentNode, higherPriorityChild);
        currentNode = higherPriorityChild;
    }
}

template <typename T>
bool PriorityQueue<T>::empty() const {
    return count == 0;
}

template <typename T>
int PriorityQueue<T>::size() const {
    return count;
}

template <typename T>
void PriorityQueue<T>::push(const T& value) {
    HeapNode* newNode = new HeapNode(value);
    if (root == NULL) {
        root = newNode;
        ++count;
        return;
    }

    Queue<HeapNode*> queue;
    queue.enqueue(root);
    while (!queue.empty()) {
        HeapNode* current = queue.dequeue();
        if (current->left == NULL) {
            current->left = newNode;
            newNode->parent = current;
            ++count;
            bubbleUp(newNode);
            return;
        }
        if (current->right == NULL) {
            current->right = newNode;
            newNode->parent = current;
            ++count;
            bubbleUp(newNode);
            return;
        }
        queue.enqueue(current->left);
        queue.enqueue(current->right);
    }
}

template <typename T>
T PriorityQueue<T>::top() const {
    if (empty()) {
        throw std::logic_error("Priority queue is empty.");
    }
    return root->value;
}

template <typename T>
T PriorityQueue<T>::pop() {
    if (empty()) {
        throw std::logic_error("Priority queue is empty.");
    }

    T highestPriority = root->value;
    if (count == 1) {
        delete root;
        root = NULL;
        count = 0;
        return highestPriority;
    }

    HeapNode* lastNode = findLastNode();
    HeapNode* parentOfLast = findParentOf(lastNode);
    root->value = lastNode->value;

    if (parentOfLast->left == lastNode) {
        parentOfLast->left = NULL;
    }
    else {
        parentOfLast->right = NULL;
    }

    delete lastNode;
    --count;
    heapifyDown(root);
    return highestPriority;
}

template <typename T>
void PriorityQueue<T>::clear() {
    if (root == NULL) {
        count = 0;
        return;
    }

    Queue<HeapNode*> queue;
    queue.enqueue(root);
    while (!queue.empty()) {
        HeapNode* current = queue.dequeue();
        if (current->left != NULL) {
            queue.enqueue(current->left);
        }
        if (current->right != NULL) {
            queue.enqueue(current->right);
        }
        delete current;
    }

    root = NULL;
    count = 0;
}

#endif
