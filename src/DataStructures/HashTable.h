// ============================================================================
//  HashTable.h - Bang bam (polynomial hash + Separate Chaining, bucket = DLL)
//  Template: cai dat trong HashTable.tpp. Ham bam polynomialHash: HashTable.cpp.
// ============================================================================
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <string>
#include "DoublyLinkedList.h"

// Ham bam da thuc (dinh nghia trong HashTable.cpp):
//   hash = s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]   (tinh bang quy tac Horner)
unsigned int polynomialHash(const char* key);

// Mot phan tu trong bucket: khoa (ma) + gia tri
template <typename V>
struct HashEntry
{
    std::string key;
    V value;

    HashEntry();
    HashEntry(const std::string& k, const V& v);

    // Hai entry bang nhau khi cung khoa -> dung cho DoublyLinkedList::find/indexOf
    bool operator==(const HashEntry<V>& other) const;
};

template <typename V>
class HashTable
{
private:
    DoublyLinkedList<HashEntry<V> >* buckets;      // mang cac bucket (cap phat bang new[])
    int capacity;                                  // so bucket
    int count;                                     // so phan tu dang luu

public:
    explicit HashTable(int bucketCount = 11);
    ~HashTable();

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Chuyen ma (chuoi) thanh chi so bucket trong [0, capacity)
    int hashFunction(const char* key) const;

    bool insert(const std::string& key, const V& value);   // O(1) trung binh
    V* search(const std::string& key) const;                // O(1) trung binh
    bool remove(const std::string& key);                    // O(1) trung binh
    bool contains(const std::string& key) const;
    void clear();                                            // xoa toan bo bucket

    int size() const;
    int getCapacity() const;
    int getBucketSize(int index) const;
    int countCollisions() const;                            // tong (do dai bucket - 1) cua bucket >= 2 phan tu

    void display() const;                                   // in bucket va collision
};

#include "HashTable.tpp"

#endif
