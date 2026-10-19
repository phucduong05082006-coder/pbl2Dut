#pragma once

#include <cstddef>
#include <utility>

template <typename T>
class LinkedList {
public:
    LinkedList() = default;
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;
    LinkedList(LinkedList&&) = delete;
    LinkedList& operator=(LinkedList&&) = delete;

    ~LinkedList() {
        clear();
    }

    void swap(LinkedList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    T& pushFront(const T& value) {
        Node* node = new Node{value, nullptr, head_};
        if (head_ != nullptr) {
            head_->previous = node;
        } else {
            tail_ = node;
        }
        head_ = node;
        ++size_;
        return node->value;
    }

    T& pushBack(const T& value) {
        Node* node = new Node{value, tail_, nullptr};
        if (tail_ != nullptr) {
            tail_->next = node;
        } else {
            head_ = node;
        }
        tail_ = node;
        ++size_;
        return node->value;
    }

    template <typename Predicate>
    T* findIf(Predicate predicate) {
        for (Node* current = head_; current != nullptr; current = current->next) {
            if (predicate(current->value)) {
                return &current->value;
            }
        }
        return nullptr;
    }

    template <typename Predicate>
    const T* findIf(Predicate predicate) const {
        for (const Node* current = head_; current != nullptr; current = current->next) {
            if (predicate(current->value)) {
                return &current->value;
            }
        }
        return nullptr;
    }

    template <typename Function>
    void forEach(Function function) {
        for (Node* current = head_; current != nullptr; current = current->next) {
            function(current->value);
        }
    }

    template <typename Function>
    void forEach(Function function) const {
        for (const Node* current = head_; current != nullptr; current = current->next) {
            function(current->value);
        }
    }

    template <typename Predicate>
    bool eraseIf(Predicate predicate) {
        for (Node* current = head_; current != nullptr; current = current->next) {
            if (!predicate(current->value)) {
                continue;
            }
            if (current->previous != nullptr) {
                current->previous->next = current->next;
            } else {
                head_ = current->next;
            }
            if (current->next != nullptr) {
                current->next->previous = current->previous;
            } else {
                tail_ = current->previous;
            }
            delete current;
            --size_;
            return true;
        }
        return false;
    }

    void clear() noexcept {
        while (head_ != nullptr) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
        tail_ = nullptr;
        size_ = 0;
    }

private:
    struct Node {
        T value;
        Node* previous;
        Node* next;
    };

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    std::size_t size_ = 0;
};
