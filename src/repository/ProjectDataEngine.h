#ifndef CINEMA_REPOSITORY_PROJECT_DATA_ENGINE_H
#define CINEMA_REPOSITORY_PROJECT_DATA_ENGINE_H

#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include "DataStructures/DoublyLinkedList.h"
#include "DataStructures/Matrix2D.h"
#include "../entity/Auditorium.h"
#include "../entity/Film.h"
#include "../entity/SaleTransaction.h"
#include "../entity/Screening.h"
#include "../entity/Seat.h"
#include "../entity/Ticket.h"
#include "../entity/User.h"
#include "txt/TxtDatabase.h"

namespace cinema
{
enum class SearchEntity
{
    Film,
    User,
    Auditorium
};

enum class SearchMode
{
    Exact,
    Prefix,
    Contains
};

struct SearchRequest
{
    SearchEntity entity = SearchEntity::Film;
    std::string text;
    SearchMode mode = SearchMode::Contains;
    int limit = 0;
};

struct SearchHit
{
    SearchEntity entity = SearchEntity::Film;
    std::string recordId;
    std::string displayText;
    int score = 0;
};

struct SearchResult
{
    DoublyLinkedList<SearchHit> hits;

    int size() const { return hits.size(); }
};

template <typename T>
struct QuerySpec
{
    std::function<bool(const T&)> where;
    std::function<bool(const T&, const T&)> orderBy;
    int limit = 0;
};

struct AggregateRow
{
    std::string key;
    long long count = 0;
    long long total = 0;
};

template <typename T>
struct AggregateSpec
{
    std::function<bool(const T&)> where;
    std::function<std::string(const T&)> groupBy;
    std::function<long long(const T&)> value;
};

class ProjectDataEngine final
{
public:
    explicit ProjectDataEngine(const std::string& rootPath);

    bool open(std::string& error);
    bool save(std::string& error) const;

    SearchResult search(const SearchRequest& request) const;

    template <typename T>
    DoublyLinkedList<T> query(const QuerySpec<T>& specification = {}) const
    {
        if (specification.limit < 0)
            throw std::invalid_argument("Query limit must be non-negative.");

        const auto& source = recordsFor(static_cast<const T*>(nullptr));
        DoublyLinkedList<T> result;
        for (const Node<T>* node = source.getHead(); node != nullptr; node = node->next)
        {
            if (specification.where && !specification.where(node->data)) continue;
            result.addLast(node->data);
            if (!specification.orderBy && specification.limit > 0 && result.size() >= specification.limit)
                break;
        }

        if (specification.orderBy)
        {
            // ponytail: O(m^2) stable insertion sort; use list merge sort if result sets grow.
            for (Node<T>* current = result.getHead(); current != nullptr; current = current->next)
            {
                T value = current->data;
                Node<T>* position = current;
                while (position->prev != nullptr && specification.orderBy(value, position->prev->data))
                {
                    position->data = position->prev->data;
                    position = position->prev;
                }
                position->data = std::move(value);
            }
        }

        if (specification.limit > 0)
            while (result.size() > specification.limit) result.removeLast();
        return result;
    }

    bool getUser(const std::string& userId, User& result) const;
    bool getUserByUsername(const std::string& username, User& result) const;
    bool getFilm(const std::string& filmId, Film& result) const;
    bool getAuditorium(const std::string& auditoriumId, Auditorium& result) const;
    bool getSeat(const std::string& seatId, Seat& result) const;
    bool getScreening(const std::string& screeningId, Screening& result) const;
    bool getTransaction(const std::string& transactionId, SaleTransaction& result) const;
    bool getTicket(const std::string& ticketId, Ticket& result) const;

    bool addUser(const User& user, std::string& error);
    bool addFilm(const Film& film, std::string& error);
    bool addAuditorium(const Auditorium& auditorium, std::string& error);
    bool addSeat(const Seat& seat, std::string& error);
    bool addScreening(const Screening& screening, std::string& error);

    bool saveUser(const User& user, std::string& error);
    bool saveFilm(const Film& film, std::string& error);
    bool saveAuditorium(const Auditorium& auditorium, const DoublyLinkedList<Seat>& seats, std::string& error);
    bool saveScreening(const Screening& screening, std::string& error);

    template <typename T>
    DoublyLinkedList<AggregateRow> aggregate(const AggregateSpec<T>& specification) const
    {
        DoublyLinkedList<AggregateRow> result;
        HashTable<Node<AggregateRow>*> groups;
        const auto& source = recordsFor(static_cast<const T*>(nullptr));
        for (const Node<T>* node = source.getHead(); node != nullptr; node = node->next)
        {
            if (specification.where && !specification.where(node->data)) continue;
            const std::string key = specification.groupBy ? specification.groupBy(node->data) : "all";
            if (key.empty()) throw std::invalid_argument("Aggregate group key must not be empty.");
            auto found = groups.search(key);
            if (found == nullptr)
            {
                result.addLast({key, 0, 0});
                groups.insert(key, result.getTail());
                found = groups.search(key);
            }
            AggregateRow& row = (*found)->data;
            const long long value = specification.value ? specification.value(node->data) : 0;
            if ((value > 0 && row.total > std::numeric_limits<long long>::max() - value) ||
                (value < 0 && row.total < std::numeric_limits<long long>::min() - value))
                throw std::overflow_error("Aggregate total overflow.");
            row.total += value;
            ++row.count;
        }
        return result;
    }

    bool commitSale(const SaleTransaction& transaction,
                    const DoublyLinkedList<Ticket>& saleTickets,
                    std::string& error);

    Matrix2D seatMapForScreening(const std::string& screeningId) const;

private:
    TxtDatabase database;

    const DoublyLinkedList<User>& recordsFor(const User*) const { return database.users(); }
    const DoublyLinkedList<Film>& recordsFor(const Film*) const { return database.films(); }
    const DoublyLinkedList<Auditorium>& recordsFor(const Auditorium*) const { return database.auditoriums(); }
    const DoublyLinkedList<Seat>& recordsFor(const Seat*) const { return database.seats(); }
    const DoublyLinkedList<Screening>& recordsFor(const Screening*) const { return database.screenings(); }
    const DoublyLinkedList<SaleTransaction>& recordsFor(const SaleTransaction*) const { return database.transactions(); }
    const DoublyLinkedList<Ticket>& recordsFor(const Ticket*) const { return database.tickets(); }
};
}

#endif
