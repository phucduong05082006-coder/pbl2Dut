// HashTable.tpp - cai dat HashEntry<V> va HashTable<V> (include o cuoi HashTable.h)

// ---------------------------------------------------------------- HashEntry
template <typename V>
HashEntry<V>::HashEntry() : key(""), value()
{
}

template <typename V>
HashEntry<V>::HashEntry(const std::string& k, const V& v) : key(k), value(v)
{
}

template <typename V>
bool HashEntry<V>::operator==(const HashEntry<V>& other) const
{
    return key == other.key;
}

// ---------------------------------------------------------------- HashTable
template <typename V>
HashTable<V>::HashTable(int bucketCount)
    : buckets(nullptr), capacity(bucketCount > 0 ? bucketCount : 11), count(0)
{
    buckets = new DoublyLinkedList<HashEntry<V> >[capacity];
}

template <typename V>
HashTable<V>::~HashTable()
{
    delete[] buckets;
}

template <typename V>
int HashTable<V>::hashFunction(const char* key) const
{
    return (int)(polynomialHash(key) % (unsigned int)capacity);
}

template <typename V>
bool HashTable<V>::insert(const std::string& key, const V& value)
{
    if (key.empty()) return false;
    HashEntry<V> entry(key, value);
    int index = hashFunction(key.c_str());
    if (buckets[index].contains(entry)) return false;       // ma trung
    buckets[index].addLast(entry);                          // collision -> noi vao chuoi
    count++;
    return true;
}

// Tra ve con tro toi value (nullptr neu khong co)
template <typename V>
V* HashTable<V>::search(const std::string& key) const
{
    HashEntry<V> probe(key, V());
    int index = hashFunction(key.c_str());
    HashEntry<V>* found = buckets[index].find(probe);
    return (found == nullptr) ? nullptr : &(found->value);
}

template <typename V>
bool HashTable<V>::remove(const std::string& key)
{
    HashEntry<V> probe(key, V());
    int index = hashFunction(key.c_str());
    int pos = buckets[index].indexOf(probe);
    if (pos == -1) return false;
    buckets[index].removeAt(pos);
    count--;
    return true;
}

template <typename V>
bool HashTable<V>::contains(const std::string& key) const
{
    return search(key) != nullptr;
}

template <typename V>
void HashTable<V>::clear()
{
    for (int index = 0; index < capacity; index++) buckets[index].clear();
    count = 0;
}

template <typename V>
int HashTable<V>::size() const { return count; }

template <typename V>
int HashTable<V>::getCapacity() const { return capacity; }

template <typename V>
int HashTable<V>::getBucketSize(int index) const
{
    return (index < 0 || index >= capacity) ? 0 : buckets[index].size();
}

template <typename V>
int HashTable<V>::countCollisions() const
{
    int total = 0;
    for (int i = 0; i < capacity; i++)
        if (buckets[i].size() > 1) total += buckets[i].size() - 1;
    return total;
}

template <typename V>
void HashTable<V>::display() const
{
    std::cout << "Bang bam: " << count << " phan tu, " << capacity << " bucket, "
              << countCollisions() << " va cham\n";
    for (int i = 0; i < capacity; i++)
    {
        std::cout << "  Bucket [" << i << "]: ";
        if (buckets[i].isEmpty()) { std::cout << "(trong)\n"; continue; }
        for (Node<HashEntry<V> >* cur = buckets[i].getHead(); cur != nullptr; cur = cur->next)
        {
            std::cout << cur->data.key;
            if (cur->next != nullptr) std::cout << " -> ";
        }
        if (buckets[i].size() > 1)
            std::cout << "    <-- COLLISION (" << buckets[i].size() << " ma cung bucket)";
        std::cout << "\n";
    }
}
