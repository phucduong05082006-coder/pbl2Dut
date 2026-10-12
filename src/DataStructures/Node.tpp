// Node.tpp - cai dat cua Node<T> (duoc #include o cuoi Node.h, KHONG bien dich rieng)

template <typename T>
Node<T>::Node(const T& value) : data(value), prev(nullptr), next(nullptr)
{
}
