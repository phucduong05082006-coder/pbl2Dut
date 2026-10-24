#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <cstddef>
#include <functional>
#include "DoublyLinkedList.h"

template <typename Key, typename Value, typename Hash = std::hash<Key> >
class HashTable {
private:
    struct Entry {
        Key key;
        Value value;

        Entry(const Key& entryKey, const Value& entryValue);
    };

    DoublyLinkedList<Entry>* buckets;
    int bucketCount;
    int elementCount;
    Hash hashFunction;

    int getBucketIndex(const Key& key) const;
    void copyFrom(const HashTable& other);
    void swap(HashTable& other);

public:
    explicit HashTable(int capacity = 11, const Hash& hashObject = Hash());
    ~HashTable();
    HashTable(const HashTable& other);
    HashTable& operator=(const HashTable& other);

    int size() const;
    bool empty() const;
    void insert(const Key& key, const Value& value);
    bool contains(const Key& key) const;
    Value* find(const Key& key);
    const Value* find(const Key& key) const;
    bool remove(const Key& key);
    void clear();
};

template <typename Key, typename Value, typename Hash>
HashTable<Key, Value, Hash>::Entry::Entry(const Key& entryKey, const Value& entryValue)
    : key(entryKey), value(entryValue) {
}

template <typename Key, typename Value, typename Hash>
HashTable<Key, Value, Hash>::HashTable(int capacity, const Hash& hashObject)
    : buckets(NULL),
      bucketCount(capacity > 0 ? capacity : 11),
      elementCount(0),
      hashFunction(hashObject) {
    buckets = new DoublyLinkedList<Entry>[bucketCount];
}

template <typename Key, typename Value, typename Hash>
HashTable<Key, Value, Hash>::~HashTable() {
    delete[] buckets;
}

template <typename Key, typename Value, typename Hash>
HashTable<Key, Value, Hash>::HashTable(const HashTable& other)
    : buckets(NULL),
      bucketCount(other.bucketCount),
      elementCount(0),
      hashFunction(other.hashFunction) {
    buckets = new DoublyLinkedList<Entry>[bucketCount];
    try {
        copyFrom(other);
    }
    catch (...) {
        delete[] buckets;
        throw;
    }
}

template <typename Key, typename Value, typename Hash>
HashTable<Key, Value, Hash>& HashTable<Key, Value, Hash>::operator=(const HashTable& other) {
    if (this != &other) {
        HashTable copy(other);
        swap(copy);
    }
    return *this;
}

template <typename Key, typename Value, typename Hash>
void HashTable<Key, Value, Hash>::swap(HashTable& other) {
    DoublyLinkedList<Entry>* temporaryBuckets = buckets;
    buckets = other.buckets;
    other.buckets = temporaryBuckets;

    int temporaryBucketCount = bucketCount;
    bucketCount = other.bucketCount;
    other.bucketCount = temporaryBucketCount;

    int temporaryElementCount = elementCount;
    elementCount = other.elementCount;
    other.elementCount = temporaryElementCount;

    Hash temporaryHash = hashFunction;
    hashFunction = other.hashFunction;
    other.hashFunction = temporaryHash;
}

template <typename Key, typename Value, typename Hash>
int HashTable<Key, Value, Hash>::getBucketIndex(const Key& key) const {
    std::size_t hashValue = hashFunction(key);
    return static_cast<int>(hashValue % static_cast<std::size_t>(bucketCount));
}

template <typename Key, typename Value, typename Hash>
void HashTable<Key, Value, Hash>::copyFrom(const HashTable& other) {
    for (int i = 0; i < other.bucketCount; ++i) {
        const DoublyLinkedList<Entry>& sourceBucket = other.buckets[i];
        for (int j = 0; j < sourceBucket.size(); ++j) {
            const Entry& entry = sourceBucket.at(j);
            buckets[i].pushBack(entry);
            ++elementCount;
        }
    }
}

template <typename Key, typename Value, typename Hash>
int HashTable<Key, Value, Hash>::size() const {
    return elementCount;
}

template <typename Key, typename Value, typename Hash>
bool HashTable<Key, Value, Hash>::empty() const {
    return elementCount == 0;
}

template <typename Key, typename Value, typename Hash>
void HashTable<Key, Value, Hash>::insert(const Key& key, const Value& value) {
    int index = getBucketIndex(key);
    DoublyLinkedList<Entry>& bucket = buckets[index];

    for (int i = 0; i < bucket.size(); ++i) {
        if (bucket.at(i).key == key) {
            bucket.at(i).value = value;
            return;
        }
    }

    bucket.pushBack(Entry(key, value));
    ++elementCount;
}

template <typename Key, typename Value, typename Hash>
bool HashTable<Key, Value, Hash>::contains(const Key& key) const {
    int index = getBucketIndex(key);
    const DoublyLinkedList<Entry>& bucket = buckets[index];

    for (int i = 0; i < bucket.size(); ++i) {
        if (bucket.at(i).key == key) {
            return true;
        }
    }
    return false;
}

template <typename Key, typename Value, typename Hash>
Value* HashTable<Key, Value, Hash>::find(const Key& key) {
    int index = getBucketIndex(key);
    DoublyLinkedList<Entry>& bucket = buckets[index];

    for (int i = 0; i < bucket.size(); ++i) {
        if (bucket.at(i).key == key) {
            return &bucket.at(i).value;
        }
    }
    return NULL;
}

template <typename Key, typename Value, typename Hash>
const Value* HashTable<Key, Value, Hash>::find(const Key& key) const {
    int index = getBucketIndex(key);
    const DoublyLinkedList<Entry>& bucket = buckets[index];

    for (int i = 0; i < bucket.size(); ++i) {
        if (bucket.at(i).key == key) {
            return &bucket.at(i).value;
        }
    }
    return NULL;
}

template <typename Key, typename Value, typename Hash>
bool HashTable<Key, Value, Hash>::remove(const Key& key) {
    int index = getBucketIndex(key);
    DoublyLinkedList<Entry>& bucket = buckets[index];

    for (int i = 0; i < bucket.size(); ++i) {
        if (bucket.at(i).key == key) {
            bucket.removeAt(i);
            --elementCount;
            return true;
        }
    }
    return false;
}

template <typename Key, typename Value, typename Hash>
void HashTable<Key, Value, Hash>::clear() {
    for (int i = 0; i < bucketCount; ++i) {
        buckets[i].clear();
    }
    elementCount = 0;
}

#endif
