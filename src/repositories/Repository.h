#pragma once

#include "DataStructures/HashTable.h"
#include "DataStructures/LinkedList.h"
#include "entities/EntityValidation.h"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <typename T>
class Repository {
public:
    using value_type = T;

    Repository() = default;
    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;
    Repository(Repository&&) = delete;
    Repository& operator=(Repository&&) = delete;

    [[nodiscard]] bool empty() const noexcept { return records_.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
    [[nodiscard]] bool isDirty() const noexcept { return dirty_; }
    void markSaved() noexcept { dirty_ = false; }

    // Publish a fully prepared replacement without allocating or reusing old IDs.
    void replaceWith(Repository& replacement) noexcept {
        if (this == &replacement) {
            return;
        }
        const EntityId previousHighest = highestId_;
        records_.swap(replacement.records_);
        index_.swap(replacement.index_);
        std::swap(highestId_, replacement.highestId_);
        std::swap(dirty_, replacement.dirty_);
        if (highestId_ < previousHighest) {
            highestId_ = previousHighest;
        }
        dirty_ = true;
    }

    EntityId nextId() const {
        if (highestId_ == std::numeric_limits<EntityId>::max()) {
            throw std::overflow_error("Entity ID exhausted");
        }
        return highestId_ + 1;
    }

    const T* findById(EntityId id) const {
        T* const* record = index_.find(id);
        return record == nullptr ? nullptr : *record;
    }

    template <typename Function>
    void forEach(Function function) const {
        records_.forEach(function);
    }

    bool insert(const T& value) {
        entity_detail::positive(value.id(), "Entity ID");
        if (index_.find(value.id()) != nullptr) {
            return false;
        }
        T& record = records_.pushBack(value);
        try {
            if (!index_.insert(record.id(), &record)) {
                records_.eraseIf([&](const T& item) { return &item == &record; });
                return false;
            }
        } catch (...) {
            records_.eraseIf([&](const T& item) { return &item == &record; });
            throw;
        }
        if (record.id() > highestId_) {
            highestId_ = record.id();
        }
        dirty_ = true;
        return true;
    }

    bool update(EntityId id, const T& value) {
        static_assert(std::is_nothrow_swappable_v<T>, "Repository update requires noexcept swap");
        if (id != value.id()) {
            throw std::invalid_argument("Cannot change an entity ID");
        }
        T** record = index_.find(id);
        if (record == nullptr) {
            return false;
        }
        T replacement(value);
        using std::swap;
        swap(**record, replacement);
        dirty_ = true;
        return true;
    }

    bool erase(EntityId id) {
        T** indexed = index_.find(id);
        if (indexed == nullptr) {
            return false;
        }
        const T* record = *indexed;
        index_.erase(id);
        records_.eraseIf([&](const T& item) { return &item == record; });
        dirty_ = true;
        return true;
    }

private:
    LinkedList<T> records_;
    HashTable<EntityId, T*> index_;
    EntityId highestId_ = 0;
    bool dirty_ = false;
};
