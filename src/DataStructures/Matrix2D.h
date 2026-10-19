#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>

template <typename T>
class Matrix2D {
public:
    Matrix2D(std::size_t rows, std::size_t columns)
        : rows_(rows), columns_(columns) {
        if (rows == 0 || columns == 0) {
            throw std::invalid_argument("Matrix dimensions must be positive");
        }
        const std::size_t maximum = std::numeric_limits<std::size_t>::max();
        if (rows > maximum / columns || rows * columns > maximum / sizeof(T)) {
            throw std::length_error("Matrix allocation size overflow");
        }
        values_ = new T[rows * columns]{};
    }

    Matrix2D(const Matrix2D&) = delete;
    Matrix2D& operator=(const Matrix2D&) = delete;
    Matrix2D(Matrix2D&&) = delete;
    Matrix2D& operator=(Matrix2D&&) = delete;

    ~Matrix2D() {
        delete[] values_;
    }

    [[nodiscard]] std::size_t rows() const noexcept {
        return rows_;
    }

    [[nodiscard]] std::size_t columns() const noexcept {
        return columns_;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return rows_ * columns_;
    }

    T& at(std::size_t row, std::size_t column) {
        checkBounds(row, column);
        return values_[row * columns_ + column];
    }

    const T& at(std::size_t row, std::size_t column) const {
        checkBounds(row, column);
        return values_[row * columns_ + column];
    }

    void fill(const T& value) {
        for (std::size_t index = 0; index < size(); ++index) {
            values_[index] = value;
        }
    }

private:
    void checkBounds(std::size_t row, std::size_t column) const {
        if (row >= rows_ || column >= columns_) {
            throw std::out_of_range("Matrix index out of range");
        }
    }

    std::size_t rows_;
    std::size_t columns_;
    T* values_ = nullptr;
};
