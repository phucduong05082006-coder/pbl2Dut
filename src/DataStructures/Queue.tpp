// Queue.tpp - cai dat Queue<T> (include o cuoi Queue.h)

template <typename T>
void Queue<T>::enqueue(const T& value)
{
    list.addLast(value);
}

template <typename T>
bool Queue<T>::dequeue(T& out)
{
    if (list.isEmpty()) return false;
    out = list.getHead()->data;
    list.removeFirst();
    return true;
}

template <typename T>
T* Queue<T>::front() const
{
    return list.isEmpty() ? nullptr : &(list.getHead()->data);
}

template <typename T>
bool Queue<T>::isEmpty() const { return list.isEmpty(); }

template <typename T>
int Queue<T>::size() const { return list.size(); }

template <typename T>
void Queue<T>::clear() { list.clear(); }

template <typename T>
void Queue<T>::display() const
{
    if (list.isEmpty()) { std::cout << "(hang doi rong)\n"; return; }
    std::cout << "Front -> ";
    for (Node<T>* cur = list.getHead(); cur != nullptr; cur = cur->next)
    {
        std::cout << "[" << cur->data << "]";
        if (cur->next != nullptr) std::cout << " -> ";
    }
    std::cout << " <- Rear\n";
}
