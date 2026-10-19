#ifdef NDEBUG
#undef NDEBUG
#endif

#include "DataStructures/Matrix2D.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

struct SeatState {
    int seatId = 0;
    bool sold = false;
};

struct TrackedCell {
    inline static int alive = 0;
    inline static int constructionsBeforeFailure = -1;

    TrackedCell() {
        if (constructionsBeforeFailure == 0) {
            throw std::runtime_error("construction failed");
        }
        if (constructionsBeforeFailure > 0) {
            --constructionsBeforeFailure;
        }
        ++alive;
    }

    ~TrackedCell() {
        --alive;
    }
};

template <typename Exception, typename Function>
void expectThrow(Function function) {
    bool threw = false;
    try {
        function();
    } catch (const Exception&) {
        threw = true;
    }
    assert(threw);
}

void testValuesAndBounds() {
    Matrix2D<int> matrix(3, 4);
    assert(matrix.rows() == 3 && matrix.columns() == 4 && matrix.size() == 12);
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        for (std::size_t column = 0; column < matrix.columns(); ++column) {
            assert(matrix.at(row, column) == 0);
            matrix.at(row, column) = static_cast<int>(row * 4 + column);
        }
    }
    const auto& readOnly = matrix;
    assert(readOnly.at(0, 0) == 0 && readOnly.at(2, 3) == 11);
    expectThrow<std::out_of_range>([&] { matrix.at(3, 0); });
    expectThrow<std::out_of_range>([&] { matrix.at(0, 4); });
    expectThrow<std::out_of_range>([&] { readOnly.at(3, 4); });
    expectThrow<std::out_of_range>([&] { matrix.at(static_cast<std::size_t>(-1), 0); });
    matrix.fill(7);
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        for (std::size_t column = 0; column < matrix.columns(); ++column) {
            assert(readOnly.at(row, column) == 7);
        }
    }
    Matrix2D<int> single(1, 1);
    single.at(0, 0) = 42;
    assert(single.size() == 1 && single.at(0, 0) == 42);
}

void testSeatMapping() {
    Matrix2D<SeatState> seats(6, 8);
    for (std::size_t row = 0; row < seats.rows(); ++row) {
        for (std::size_t column = 0; column < seats.columns(); ++column) {
            SeatState& seat = seats.at(row, column);
            assert(seat.seatId == 0 && !seat.sold);
            seat.seatId = static_cast<int>(row * 8 + column + 1);
        }
    }
    seats.at(1, 2).sold = true;
    const auto& readOnly = seats;
    assert(readOnly.at(1, 2).seatId == 11 && readOnly.at(1, 2).sold);
    assert(readOnly.at(0, 0).seatId == 1 && !readOnly.at(0, 0).sold);
    assert(readOnly.at(5, 7).seatId == 48);
    seats.fill(SeatState{3, true});
    assert(seats.at(0, 0).seatId == 3 && seats.at(5, 7).sold);
}

void testInvalidDimensionsAndLifetime() {
    expectThrow<std::invalid_argument>([] { Matrix2D<int> invalid(0, 2); });
    expectThrow<std::invalid_argument>([] { Matrix2D<int> invalid(2, 0); });
    const std::size_t maximum = std::numeric_limits<std::size_t>::max();
    expectThrow<std::length_error>([&] { Matrix2D<int> invalid(maximum, 2); });
    expectThrow<std::length_error>([&] { Matrix2D<int> invalid(maximum, 1); });
    assert(TrackedCell::alive == 0);
    TrackedCell::constructionsBeforeFailure = 2;
    expectThrow<std::runtime_error>([] { Matrix2D<TrackedCell> invalid(2, 3); });
    assert(TrackedCell::alive == 0);
    TrackedCell::constructionsBeforeFailure = -1;
    {
        Matrix2D<TrackedCell> matrix(2, 3);
        assert(TrackedCell::alive == 6);
    }
    assert(TrackedCell::alive == 0);
}

int main() {
    static_assert(!std::is_copy_constructible_v<Matrix2D<int>>);
    static_assert(!std::is_copy_assignable_v<Matrix2D<int>>);
    static_assert(!std::is_move_constructible_v<Matrix2D<int>>);
    static_assert(!std::is_move_assignable_v<Matrix2D<int>>);
    static_assert(std::is_same_v<decltype(std::declval<const Matrix2D<int>&>().at(0, 0)), const int&>);
    testValuesAndBounds();
    testSeatMapping();
    testInvalidDimensionsAndLifetime();
    std::cout << "Matrix2D checks passed\n";
}
