// Stack.tpp - cai dat Stack<T> (include o cuoi Stack.h)

template <typename T>
void Stack<T>::push(const T& value)
{
    list.addLast(value);
}

template <typename T>
bool Stack<T>::pop(T& out)
{
    if (list.isEmpty()) return false;
    out = list.getTail()->data;
    list.removeLast();
    return true;
}

template <typename T>
T* Stack<T>::top() const
{
    return list.isEmpty() ? nullptr : &(list.getTail()->data);
}

template <typename T>
bool Stack<T>::isEmpty() const { return list.isEmpty(); }

template <typename T>
int Stack<T>::size() const { return list.size(); }

template <typename T>
void Stack<T>::clear() { list.clear(); }

template <typename T>
void Stack<T>::display() const
{
    if (list.isEmpty()) { std::cout << "(ngan xep rong)\n"; return; }
    std::cout << "  TOP\n";
    int position = list.size();
    for (Node<T>* cur = list.getTail(); cur != nullptr; cur = cur->prev, position--)
        std::cout << "   [" << position << "] " << cur->data << "\n";
    std::cout << "  BOTTOM\n";
}
