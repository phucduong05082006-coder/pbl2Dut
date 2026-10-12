// DoublyLinkedList.tpp - cai dat DoublyLinkedList<T> (include o cuoi DoublyLinkedList.h)

// ---------------------------------------------------------------- private
// Tim node o vi tri index. Di tu dau hoac tu cuoi tuy vi tri gan ben nao hon.
template <typename T>
Node<T>* DoublyLinkedList<T>::nodeAt(int index) const
{
    if (index < 0 || index >= count) return nullptr;
    if (index < count / 2)
    {
        Node<T>* cur = head;
        for (int i = 0; i < index; i++) cur = cur->next;
        return cur;
    }
    Node<T>* cur = tail;
    for (int i = count - 1; i > index; i--) cur = cur->prev;
    return cur;
}

// Go 1 node khoi danh sach roi giai phong bo nho cua node do.
template <typename T>
void DoublyLinkedList<T>::unlink(Node<T>* node)
{
    if (node->prev != nullptr) node->prev->next = node->next;
    else head = node->next;                        // node la head

    if (node->next != nullptr) node->next->prev = node->prev;
    else tail = node->prev;                        // node la tail

    delete node;
    count--;
}

template <typename T>
void DoublyLinkedList<T>::copyFrom(const DoublyLinkedList<T>& other)
{
    for (Node<T>* cur = other.head; cur != nullptr; cur = cur->next)
        addLast(cur->data);
}

// ---------------------------------------------------------------- public
template <typename T>
DoublyLinkedList<T>::DoublyLinkedList() : head(nullptr), tail(nullptr), count(0)
{
}

template <typename T>
DoublyLinkedList<T>::DoublyLinkedList(const DoublyLinkedList<T>& other)
    : head(nullptr), tail(nullptr), count(0)
{
    copyFrom(other);
}

template <typename T>
DoublyLinkedList<T>& DoublyLinkedList<T>::operator=(const DoublyLinkedList<T>& other)
{
    if (this != &other)
    {
        clear();
        copyFrom(other);
    }
    return *this;
}

template <typename T>
DoublyLinkedList<T>::~DoublyLinkedList()
{
    clear();
}

template <typename T>
void DoublyLinkedList<T>::addFirst(const T& value)
{
    Node<T>* node = new Node<T>(value);
    node->next = head;
    if (head != nullptr) head->prev = node;
    else tail = node;                              // danh sach dang rong
    head = node;
    count++;
}

template <typename T>
void DoublyLinkedList<T>::addLast(const T& value)
{
    Node<T>* node = new Node<T>(value);
    node->prev = tail;
    if (tail != nullptr) tail->next = node;
    else head = node;                              // danh sach dang rong
    tail = node;
    count++;
}

template <typename T>
bool DoublyLinkedList<T>::removeFirst()
{
    if (head == nullptr) return false;
    unlink(head);
    return true;
}

template <typename T>
bool DoublyLinkedList<T>::removeLast()
{
    if (tail == nullptr) return false;
    unlink(tail);
    return true;
}

template <typename T>
bool DoublyLinkedList<T>::removeAt(int index)
{
    Node<T>* node = nodeAt(index);
    if (node == nullptr) return false;
    unlink(node);
    return true;
}

template <typename T>
void DoublyLinkedList<T>::clear()
{
    while (head != nullptr) removeFirst();
}

// Tra ve con tro toi du lieu ben trong node (nullptr neu index sai)
template <typename T>
T* DoublyLinkedList<T>::getAt(int index) const
{
    Node<T>* node = nodeAt(index);
    return (node == nullptr) ? nullptr : &(node->data);
}

template <typename T>
Node<T>* DoublyLinkedList<T>::findNode(const T& value) const
{
    for (Node<T>* cur = head; cur != nullptr; cur = cur->next)
        if (cur->data == value) return cur;
    return nullptr;
}

template <typename T>
T* DoublyLinkedList<T>::find(const T& value) const
{
    Node<T>* node = findNode(value);
    return (node == nullptr) ? nullptr : &(node->data);
}

template <typename T>
int DoublyLinkedList<T>::indexOf(const T& value) const
{
    int index = 0;
    for (Node<T>* cur = head; cur != nullptr; cur = cur->next, index++)
        if (cur->data == value) return index;
    return -1;
}

template <typename T>
bool DoublyLinkedList<T>::contains(const T& value) const
{
    return indexOf(value) != -1;
}

template <typename T>
bool DoublyLinkedList<T>::isEmpty() const { return count == 0; }

template <typename T>
int DoublyLinkedList<T>::size() const { return count; }

template <typename T>
Node<T>* DoublyLinkedList<T>::getHead() const { return head; }

template <typename T>
Node<T>* DoublyLinkedList<T>::getTail() const { return tail; }

template <typename T>
void DoublyLinkedList<T>::forEach(void (*action)(const T&)) const
{
    for (Node<T>* cur = head; cur != nullptr; cur = cur->next) action(cur->data);
}

template <typename T>
void DoublyLinkedList<T>::forEachReverse(void (*action)(const T&)) const
{
    for (Node<T>* cur = tail; cur != nullptr; cur = cur->prev) action(cur->data);
}

template <typename T>
void DoublyLinkedList<T>::display(const char* separator) const
{
    if (head == nullptr) { std::cout << "(rong)\n"; return; }
    for (Node<T>* cur = head; cur != nullptr; cur = cur->next)
    {
        std::cout << cur->data;
        if (cur->next != nullptr) std::cout << separator;
    }
    std::cout << "\n";
}

template <typename T>
void DoublyLinkedList<T>::displayReverse(const char* separator) const
{
    if (tail == nullptr) { std::cout << "(rong)\n"; return; }
    for (Node<T>* cur = tail; cur != nullptr; cur = cur->prev)
    {
        std::cout << cur->data;
        if (cur->prev != nullptr) std::cout << separator;
    }
    std::cout << "\n";
}
