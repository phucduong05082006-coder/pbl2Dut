#ifdef NDEBUG
#undef NDEBUG
#endif

#include "DataStructures/LinkedList.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

struct FilmSample {
    int id;
    std::string title;
};

struct TicketSample {
    int id;
    int price;
};

struct Tracked {
    inline static int alive = 0;
    inline static bool failCopy = false;
    int id;

    explicit Tracked(int identifier) : id(identifier) {
        ++alive;
    }

    Tracked(const Tracked& other) : id(other.id) {
        if (failCopy) {
            throw std::runtime_error("copy failed");
        }
        ++alive;
    }

    ~Tracked() {
        --alive;
    }
};

void expectValues(const LinkedList<int>& list, const int* expected, std::size_t count) {
    assert(list.size() == count);
    assert(list.empty() == (count == 0));
    std::size_t index = 0;
    list.forEach([&](const int& value) {
        assert(index < count);
        assert(value == expected[index]);
        ++index;
    });
    assert(index == count);
}

void testInsertFindAndErase() {
    LinkedList<int> list;
    expectValues(list, nullptr, 0);
    assert(list.findIf([](int) { return true; }) == nullptr);
    assert(!list.eraseIf([](int) { return true; }));

    int* original = &list.pushBack(2);
    list.pushFront(1);
    list.pushBack(3);
    list.pushBack(4);
    const int initial[] = {1, 2, 3, 4};
    expectValues(list, initial, 4);
    assert(list.findIf([](int value) { return value == 2; }) == original);
    assert(list.findIf([](int value) { return value == 99; }) == nullptr);

    assert(list.eraseIf([](int value) { return value == 1; }));
    const int afterHead[] = {2, 3, 4};
    expectValues(list, afterHead, 3);
    assert(list.eraseIf([](int value) { return value == 3; }));
    const int afterMiddle[] = {2, 4};
    expectValues(list, afterMiddle, 2);
    assert(list.eraseIf([](int value) { return value == 4; }));
    assert(list.findIf([](int value) { return value == 2; }) == original);
    assert(list.eraseIf([](int value) { return value == 2; }));
    expectValues(list, nullptr, 0);

    list.pushFront(5);
    list.pushBack(6);
    list.forEach([](int& value) { value += 1; });
    const int changed[] = {6, 7};
    expectValues(list, changed, 2);
    const LinkedList<int>& readOnly = list;
    assert(*readOnly.findIf([](int value) { return value == 7; }) == 7);
    assert(readOnly.findIf([](int value) { return value == 99; }) == nullptr);
    list.clear();
    list.clear();
    expectValues(list, nullptr, 0);

    list.pushBack(8);
    list.pushBack(8);
    assert(list.eraseIf([](int value) { return value == 8; }));
    const int duplicate[] = {8};
    expectValues(list, duplicate, 1);
    list.pushBack(9);
    list.pushFront(7);
    const int reused[] = {7, 8, 9};
    expectValues(list, reused, 3);
}

void testEntityTemplates() {
    LinkedList<FilmSample> films;
    films.pushBack({1, "Film one"});
    films.pushFront({2, "Film two"});
    const FilmSample* film = films.findIf([](const FilmSample& item) { return item.id == 1; });
    assert(film != nullptr && film->title == "Film one");

    LinkedList<TicketSample> tickets;
    tickets.pushBack({1, 80000});
    tickets.pushBack({2, 90000});
    int revenue = 0;
    tickets.forEach([&](const TicketSample& ticket) { revenue += ticket.price; });
    assert(revenue == 170000);
}

void testLifetimeAndExceptions() {
    assert(Tracked::alive == 0);
    {
        Tracked value(1);
        LinkedList<Tracked> list;
        Tracked* original = &list.pushBack(value);
        assert(Tracked::alive == 2);
        const bool positions[] = {false, true};
        for (bool atFront : positions) {
            Tracked::failCopy = true;
            bool threw = false;
            try {
                if (atFront) {
                    list.pushFront(value);
                } else {
                    list.pushBack(value);
                }
            } catch (const std::runtime_error&) {
                threw = true;
            }
            Tracked::failCopy = false;
            assert(threw);
            assert(list.size() == 1 && Tracked::alive == 2);
            assert(list.findIf([](const Tracked&) { return true; }) == original);
        }
        list.pushFront(value);
        assert(Tracked::alive == 3);
        assert(list.eraseIf([](const Tracked&) { return true; }));
        assert(Tracked::alive == 2);
    }
    assert(Tracked::alive == 0);
    {
        LinkedList<Tracked> list;
        list.pushBack(Tracked(2));
        list.pushBack(Tracked(3));
        list.clear();
        assert(list.empty() && Tracked::alive == 0);
        list.pushBack(Tracked(4));
    }
    assert(Tracked::alive == 0);
}

void testRepeatedReuse() {
    LinkedList<int> list;
    for (int value = 0; value < 10000; ++value) {
        list.pushBack(value);
    }
    for (int value = 0; value < 10000; ++value) {
        assert(list.eraseIf([&](int item) { return item == value; }));
    }
    assert(list.empty());
    list.pushFront(1);
    assert(list.eraseIf([](int) { return true; }));
    list.pushBack(2);
    list.pushFront(1);
    const int expected[] = {1, 2};
    expectValues(list, expected, 2);
}

int main() {
    static_assert(!std::is_copy_constructible_v<LinkedList<int>>);
    static_assert(!std::is_copy_assignable_v<LinkedList<int>>);
    static_assert(!std::is_move_constructible_v<LinkedList<int>>);
    static_assert(!std::is_move_assignable_v<LinkedList<int>>);
    static_assert(std::is_same_v<decltype(std::declval<const LinkedList<int>&>().findIf(
        static_cast<bool (*)(int)>(nullptr))), const int*>);
    testInsertFindAndErase();
    testEntityTemplates();
    testLifetimeAndExceptions();
    testRepeatedReuse();
    std::cout << "LinkedList checks passed\n";
}
