#pragma once

#include "LinkedList.h"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

template <typename Key>
struct DefaultHash {
    std::size_t operator()(const Key& key) const noexcept {
        static_assert(std::is_integral_v<Key>, "Provide a hasher for non-integral keys");
        return static_cast<std::size_t>(key);
    }
};

template <>
struct DefaultHash<std::string> {
    std::size_t operator()(const std::string& key) const noexcept {
        std::size_t hash = 0;
        for (unsigned char character : key) {
            hash = hash * 31 + character;
        }
        return hash;
    }
};

template <typename Key, typename Value, typename Hasher = DefaultHash<Key>>
class HashTable {
public:
    explicit HashTable(std::size_t bucketCount = 8, const Hasher& hasher = Hasher{})
        : hasher_(hasher), bucketCount_(bucketCount), buckets_(allocateBuckets(bucketCount)) {
    }

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&&) = delete;
    HashTable& operator=(HashTable&&) = delete;

    ~HashTable() {
        delete[] buckets_;
    }

    void swap(HashTable& other) noexcept(std::is_nothrow_swappable_v<Hasher>) {
        using std::swap;
        swap(hasher_, other.hasher_);
        swap(bucketCount_, other.bucketCount_);
        swap(buckets_, other.buckets_);
        swap(size_, other.size_);
    }

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] std::size_t bucketCount() const noexcept {
        return bucketCount_;
    }

    Value* find(const Key& key) {
        Entry* entry = buckets_[hasher_(key) % bucketCount_].findIf(
            [&](const Entry& item) { return item.key == key; });
        return entry == nullptr ? nullptr : &entry->value;
    }

    const Value* find(const Key& key) const {
        const Bucket& bucket = buckets_[hasher_(key) % bucketCount_];
        const Entry* entry = bucket.findIf(
            [&](const Entry& item) { return item.key == key; });
        return entry == nullptr ? nullptr : &entry->value;
    }

    bool insert(const Key& key, const Value& value) {
        if (find(key) != nullptr) {
            return false;
        }
        Entry candidate{key, value};
        if (size_ == bucketCount_) {
            if (bucketCount_ > std::numeric_limits<std::size_t>::max() / 2) {
                throw std::length_error("Hash table capacity overflow");
            }
            rehash(bucketCount_ * 2);
        }
        buckets_[hasher_(candidate.key) % bucketCount_].pushBack(candidate);
        ++size_;
        return true;
    }

    bool erase(const Key& key) {
        const bool removed = buckets_[hasher_(key) % bucketCount_].eraseIf(
            [&](const Entry& item) { return item.key == key; });
        if (removed) {
            --size_;
        }
        return removed;
    }

    template <typename Function>
    void forEach(Function function) {
        for (std::size_t index = 0; index < bucketCount_; ++index) {
            buckets_[index].forEach([&](Entry& entry) {
                function(static_cast<const Key&>(entry.key), entry.value);
            });
        }
    }

    template <typename Function>
    void forEach(Function function) const {
        for (std::size_t index = 0; index < bucketCount_; ++index) {
            const Bucket& bucket = buckets_[index];
            bucket.forEach([&](const Entry& entry) {
                function(entry.key, entry.value);
            });
        }
    }

    void rehash(std::size_t newBucketCount) {
        if (newBucketCount < size_) {
            throw std::invalid_argument("Bucket count cannot be smaller than size");
        }
        if (newBucketCount == bucketCount_) {
            return;
        }
        Bucket* replacement = allocateBuckets(newBucketCount);
        try {
            forEach([&](const Key& key, const Value& value) {
                replacement[hasher_(key) % newBucketCount].pushBack(Entry{key, value});
            });
        } catch (...) {
            delete[] replacement;
            throw;
        }
        delete[] buckets_;
        buckets_ = replacement;
        bucketCount_ = newBucketCount;
    }

    void clear() noexcept {
        for (std::size_t index = 0; index < bucketCount_; ++index) {
            buckets_[index].clear();
        }
        size_ = 0;
    }

private:
    struct Entry {
        Key key;
        Value value;
    };

    using Bucket = LinkedList<Entry>;

    static Bucket* allocateBuckets(std::size_t count) {
        if (count == 0) {
            throw std::invalid_argument("Bucket count must be positive");
        }
        if (count > std::numeric_limits<std::size_t>::max() / sizeof(Bucket)) {
            throw std::length_error("Hash table allocation size overflow");
        }
        return new Bucket[count];
    }

    Hasher hasher_;
    std::size_t bucketCount_;
    Bucket* buckets_;
    std::size_t size_ = 0;
};
