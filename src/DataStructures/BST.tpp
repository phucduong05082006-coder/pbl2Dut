// BST.tpp - cai dat BSTNode<T> va BST<T> (include o cuoi BST.h)

template <typename T>
BSTNode<T>::BSTNode(const T& value) : data(value), left(nullptr), right(nullptr)
{
}

// ---------------------------------------------------------------- private
template <typename T>
BSTNode<T>* BST<T>::insertNode(BSTNode<T>* node, const T& value, bool& inserted)
{
    if (node == nullptr)
    {
        inserted = true;
        return new BSTNode<T>(value);
    }
    int c = compare(value, node->data);
    if (c < 0) node->left = insertNode(node->left, value, inserted);
    else if (c > 0) node->right = insertNode(node->right, value, inserted);
    // c == 0: trung khoa -> khong them (inserted van la false)
    return node;
}

template <typename T>
BSTNode<T>* BST<T>::removeNode(BSTNode<T>* node, const T& key, bool& removed)
{
    if (node == nullptr) return nullptr;
    int c = compare(key, node->data);
    if (c < 0)
    {
        node->left = removeNode(node->left, key, removed);
    }
    else if (c > 0)
    {
        node->right = removeNode(node->right, key, removed);
    }
    else
    {
        removed = true;
        if (node->left == nullptr)                  // 0 hoac 1 con (phai)
        {
            BSTNode<T>* rightChild = node->right;
            delete node;
            return rightChild;
        }
        if (node->right == nullptr)                 // 1 con (trai)
        {
            BSTNode<T>* leftChild = node->left;
            delete node;
            return leftChild;
        }
        // 2 con: thay bang phan tu nho nhat cua cay con phai (successor)
        BSTNode<T>* successor = node->right;
        while (successor->left != nullptr) successor = successor->left;
        T successorData = successor->data;          // copy truoc khi xoa node
        node->data = successorData;
        bool dummy = false;
        node->right = removeNode(node->right, successorData, dummy);
    }
    return node;
}

template <typename T>
void BST<T>::destroy(BSTNode<T>* node)
{
    if (node == nullptr) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

template <typename T>
void BST<T>::inorderRec(BSTNode<T>* node, VisitFunc visit) const
{
    if (node == nullptr) return;
    inorderRec(node->left, visit);                  // Left
    visit(node->data);                              // Node
    inorderRec(node->right, visit);                 // Right
}

template <typename T>
void BST<T>::preorderRec(BSTNode<T>* node, VisitFunc visit) const
{
    if (node == nullptr) return;
    visit(node->data);                              // Node
    preorderRec(node->left, visit);                 // Left
    preorderRec(node->right, visit);                // Right
}

template <typename T>
void BST<T>::postorderRec(BSTNode<T>* node, VisitFunc visit) const
{
    if (node == nullptr) return;
    postorderRec(node->left, visit);                // Left
    postorderRec(node->right, visit);               // Right
    visit(node->data);                              // Node
}

template <typename T>
int BST<T>::heightRec(BSTNode<T>* node) const
{
    if (node == nullptr) return 0;
    int leftHeight = heightRec(node->left);
    int rightHeight = heightRec(node->right);
    return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

// Ve cay xoay ngang: cay con phai o tren, cay con trai o duoi, goc o ben trai
template <typename T>
void BST<T>::printRec(BSTNode<T>* node, int depth, LabelFunc label) const
{
    if (node == nullptr) return;
    printRec(node->right, depth + 1, label);
    for (int i = 0; i < depth; i++) std::cout << "        ";
    std::cout << label(node->data) << "\n";
    printRec(node->left, depth + 1, label);
}

// ---------------------------------------------------------------- public
template <typename T>
BST<T>::BST(CompareFunc cmp) : root(nullptr), count(0), compare(cmp)
{
}

template <typename T>
BST<T>::~BST()
{
    clear();
}

template <typename T>
bool BST<T>::insert(const T& value)
{
    bool inserted = false;
    root = insertNode(root, value, inserted);
    if (inserted) count++;
    return inserted;
}

template <typename T>
T* BST<T>::search(const T& key, int* steps) const
{
    int compareCount = 0;
    BSTNode<T>* cur = root;
    while (cur != nullptr)
    {
        compareCount++;
        int c = compare(key, cur->data);
        if (c == 0) break;
        cur = (c < 0) ? cur->left : cur->right;
    }
    if (steps != nullptr) *steps = compareCount;
    return (cur == nullptr) ? nullptr : &(cur->data);
}

template <typename T>
bool BST<T>::remove(const T& key)
{
    bool removed = false;
    root = removeNode(root, key, removed);
    if (removed) count--;
    return removed;
}

template <typename T>
void BST<T>::inorder(VisitFunc visit) const { inorderRec(root, visit); }

template <typename T>
void BST<T>::preorder(VisitFunc visit) const { preorderRec(root, visit); }

template <typename T>
void BST<T>::postorder(VisitFunc visit) const { postorderRec(root, visit); }

template <typename T>
void BST<T>::printTree(LabelFunc label) const
{
    if (root == nullptr) { std::cout << "(cay rong)\n"; return; }
    printRec(root, 0, label);
}

template <typename T>
int BST<T>::size() const { return count; }

template <typename T>
int BST<T>::height() const { return heightRec(root); }

template <typename T>
bool BST<T>::isEmpty() const { return count == 0; }

template <typename T>
void BST<T>::clear()
{
    destroy(root);
    root = nullptr;
    count = 0;
}
