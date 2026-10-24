#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

template <typename T>
class BinarySearchTree {
private:
    struct Node {
        T value;
        Node* left;
        Node* right;

        explicit Node(const T& itemValue);
    };

    Node* root;
    bool (*compareFunction)(const T&, const T&);

    static bool defaultCompare(const T& leftValue, const T& rightValue);
    Node* searchNode(Node* currentNode, const T& value) const;
    void insertNode(Node*& currentNode, const T& value);
    void removeNode(Node*& currentNode, const T& value);
    Node* findMin(Node* currentNode) const;
    void traversePreOrder(Node* currentNode, void (*callback)(const T& value)) const;
    void traverseInOrder(Node* currentNode, void (*callback)(const T& value)) const;
    void traversePostOrder(Node* currentNode, void (*callback)(const T& value)) const;
    int countNodes(Node* currentNode) const;
    int treeHeight(Node* currentNode) const;
    void deleteTree(Node* currentNode);

public:
    explicit BinarySearchTree(bool (*comparisonFunction)(const T&, const T&) = defaultCompare);
    ~BinarySearchTree();
    BinarySearchTree(const BinarySearchTree&) = delete;
    BinarySearchTree& operator=(const BinarySearchTree&) = delete;

    bool insert(const T& value);
    bool contains(const T& value) const;
    bool remove(const T& value);
    void traversePreOrder(void (*callback)(const T& value)) const;
    void traverseInOrder(void (*callback)(const T& value)) const;
    void traversePostOrder(void (*callback)(const T& value)) const;
    int size() const;
    int height() const;
    bool isEmpty() const;
    void clear();
};

template <typename T>
BinarySearchTree<T>::Node::Node(const T& itemValue)
    : value(itemValue), left(NULL), right(NULL) {
}

template <typename T>
bool BinarySearchTree<T>::defaultCompare(const T& leftValue, const T& rightValue) {
    return leftValue < rightValue;
}

template <typename T>
BinarySearchTree<T>::BinarySearchTree(bool (*comparisonFunction)(const T&, const T&))
    : root(NULL), compareFunction(comparisonFunction) {
}

template <typename T>
BinarySearchTree<T>::~BinarySearchTree() {
    clear();
}

template <typename T>
typename BinarySearchTree<T>::Node* BinarySearchTree<T>::searchNode(
    Node* currentNode, const T& value) const {
    if (currentNode == NULL) {
        return NULL;
    }
    if (value == currentNode->value) {
        return currentNode;
    }
    if (compareFunction(value, currentNode->value)) {
        return searchNode(currentNode->left, value);
    }
    return searchNode(currentNode->right, value);
}

template <typename T>
void BinarySearchTree<T>::insertNode(Node*& currentNode, const T& value) {
    if (currentNode == NULL) {
        currentNode = new Node(value);
        return;
    }
    if (value == currentNode->value) {
        return;
    }
    if (compareFunction(value, currentNode->value)) {
        insertNode(currentNode->left, value);
    }
    else {
        insertNode(currentNode->right, value);
    }
}

template <typename T>
void BinarySearchTree<T>::removeNode(Node*& currentNode, const T& value) {
    if (currentNode == NULL) {
        return;
    }
    if (!(value == currentNode->value)) {
        if (compareFunction(value, currentNode->value)) {
            removeNode(currentNode->left, value);
        }
        else {
            removeNode(currentNode->right, value);
        }
        return;
    }

    if (currentNode->left == NULL && currentNode->right == NULL) {
        delete currentNode;
        currentNode = NULL;
        return;
    }
    if (currentNode->left == NULL) {
        Node* replacement = currentNode->right;
        delete currentNode;
        currentNode = replacement;
        return;
    }
    if (currentNode->right == NULL) {
        Node* replacement = currentNode->left;
        delete currentNode;
        currentNode = replacement;
        return;
    }

    Node* minNode = findMin(currentNode->right);
    currentNode->value = minNode->value;
    removeNode(currentNode->right, minNode->value);
}

template <typename T>
typename BinarySearchTree<T>::Node* BinarySearchTree<T>::findMin(Node* currentNode) const {
    while (currentNode != NULL && currentNode->left != NULL) {
        currentNode = currentNode->left;
    }
    return currentNode;
}

template <typename T>
void BinarySearchTree<T>::traversePreOrder(
    Node* currentNode, void (*callback)(const T& value)) const {
    if (currentNode == NULL) {
        return;
    }
    callback(currentNode->value);
    traversePreOrder(currentNode->left, callback);
    traversePreOrder(currentNode->right, callback);
}

template <typename T>
void BinarySearchTree<T>::traverseInOrder(
    Node* currentNode, void (*callback)(const T& value)) const {
    if (currentNode == NULL) {
        return;
    }
    traverseInOrder(currentNode->left, callback);
    callback(currentNode->value);
    traverseInOrder(currentNode->right, callback);
}

template <typename T>
void BinarySearchTree<T>::traversePostOrder(
    Node* currentNode, void (*callback)(const T& value)) const {
    if (currentNode == NULL) {
        return;
    }
    traversePostOrder(currentNode->left, callback);
    traversePostOrder(currentNode->right, callback);
    callback(currentNode->value);
}

template <typename T>
int BinarySearchTree<T>::countNodes(Node* currentNode) const {
    if (currentNode == NULL) {
        return 0;
    }
    return 1 + countNodes(currentNode->left) + countNodes(currentNode->right);
}

template <typename T>
int BinarySearchTree<T>::treeHeight(Node* currentNode) const {
    if (currentNode == NULL) {
        return 0;
    }

    int leftHeight = treeHeight(currentNode->left);
    int rightHeight = treeHeight(currentNode->right);
    return (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
}

template <typename T>
void BinarySearchTree<T>::deleteTree(Node* currentNode) {
    if (currentNode == NULL) {
        return;
    }
    deleteTree(currentNode->left);
    deleteTree(currentNode->right);
    delete currentNode;
}

template <typename T>
bool BinarySearchTree<T>::insert(const T& value) {
    if (contains(value)) {
        return false;
    }
    insertNode(root, value);
    return true;
}

template <typename T>
bool BinarySearchTree<T>::contains(const T& value) const {
    return searchNode(root, value) != NULL;
}

template <typename T>
bool BinarySearchTree<T>::remove(const T& value) {
    if (!contains(value)) {
        return false;
    }
    removeNode(root, value);
    return true;
}

template <typename T>
void BinarySearchTree<T>::traversePreOrder(void (*callback)(const T& value)) const {
    traversePreOrder(root, callback);
}

template <typename T>
void BinarySearchTree<T>::traverseInOrder(void (*callback)(const T& value)) const {
    traverseInOrder(root, callback);
}

template <typename T>
void BinarySearchTree<T>::traversePostOrder(void (*callback)(const T& value)) const {
    traversePostOrder(root, callback);
}

template <typename T>
int BinarySearchTree<T>::size() const {
    return countNodes(root);
}

template <typename T>
int BinarySearchTree<T>::height() const {
    return treeHeight(root);
}

template <typename T>
bool BinarySearchTree<T>::isEmpty() const {
    return root == NULL;
}

template <typename T>
void BinarySearchTree<T>::clear() {
    deleteTree(root);
    root = NULL;
}

#endif
