#include "ProjectDataEngine.h"

#include <cctype>

namespace
{
using cinema::SearchMode;

std::string normalizeAscii(const std::string& value)
{
    std::string result;
    result.reserve(value.size());
    for (unsigned char character : value)
        result.push_back(static_cast<char>(std::tolower(character)));
    return result;
}

bool matches(const std::string& candidate, const std::string& query, SearchMode mode, int& score)
{
    const std::string normalizedCandidate = normalizeAscii(candidate);
    const std::string normalizedQuery = normalizeAscii(query);

    if (mode == SearchMode::Exact && normalizedCandidate == normalizedQuery)
    {
        score = 100;
        return true;
    }
    if (mode == SearchMode::Prefix && normalizedCandidate.rfind(normalizedQuery, 0) == 0)
    {
        score = 90;
        return true;
    }
    if (mode == SearchMode::Contains && normalizedCandidate.find(normalizedQuery) != std::string::npos)
    {
        score = 70;
        return true;
    }
    return false;
}

bool reachedLimit(const cinema::SearchResult& result, int limit)
{
    return limit > 0 && result.size() >= limit;
}
}

namespace cinema
{
ProjectDataEngine::ProjectDataEngine(const std::string& rootPath)
    : database(rootPath)
{
}

bool ProjectDataEngine::open(std::string& error)
{
    return database.load(error);
}

bool ProjectDataEngine::save(std::string& error) const
{
    return database.saveAll(error);
}

SearchResult ProjectDataEngine::search(const SearchRequest& request) const
{
    if (request.limit < 0 ||
        (request.entity != SearchEntity::Film && request.entity != SearchEntity::User && request.entity != SearchEntity::Auditorium) ||
        (request.mode != SearchMode::Exact && request.mode != SearchMode::Prefix && request.mode != SearchMode::Contains))
        throw std::invalid_argument("Invalid search request.");
    SearchResult result;

    if (request.text.empty()) return result;

    if (request.entity == SearchEntity::Film)
    {
        for (Node<Film>* node = database.films().getHead(); node != nullptr; node = node->next)
        {
            int score = 0;
            const Film& film = node->data;
            if (matches(film.title, request.text, request.mode, score) ||
                matches(film.filmId, request.text, request.mode, score))
            {
                result.hits.addLast({SearchEntity::Film, film.filmId, film.title, score});
            }
            if (reachedLimit(result, request.limit)) break;
        }
    }
    else if (request.entity == SearchEntity::User)
    {
        for (Node<User>* node = database.users().getHead(); node != nullptr; node = node->next)
        {
            int score = 0;
            const User& user = node->data;
            if (matches(user.username, request.text, request.mode, score) ||
                matches(user.displayName, request.text, request.mode, score) ||
                matches(user.userId, request.text, request.mode, score))
            {
                result.hits.addLast({SearchEntity::User, user.userId, user.displayName, score});
            }
            if (reachedLimit(result, request.limit)) break;
        }
    }
    else if (request.entity == SearchEntity::Auditorium)
    {
        for (Node<Auditorium>* node = database.auditoriums().getHead(); node != nullptr; node = node->next)
        {
            int score = 0;
            const Auditorium& auditorium = node->data;
            if (matches(auditorium.name, request.text, request.mode, score) ||
                matches(auditorium.auditoriumId, request.text, request.mode, score))
            {
                result.hits.addLast({SearchEntity::Auditorium, auditorium.auditoriumId, auditorium.name, score});
            }
            if (reachedLimit(result, request.limit)) break;
        }
    }

    return result;
}

bool ProjectDataEngine::getUser(const std::string& userId, User& result) const
{
    const User* value = database.findUser(userId);
    if (value == nullptr) return false;
    result = *value;
    return true;
}

bool ProjectDataEngine::getUserByUsername(const std::string& username, User& result) const
{
    const User* value = database.findUserByUsername(username);
    if (value == nullptr) return false;
    result = *value;
    return true;
}

bool ProjectDataEngine::getFilm(const std::string& filmId, Film& result) const
{
    const Film* value = database.findFilm(filmId);
    if (value == nullptr) return false;
    result = *value;
    return true;
}

bool ProjectDataEngine::getAuditorium(const std::string& auditoriumId, Auditorium& result) const
{
    const Auditorium* value = database.findAuditorium(auditoriumId);
    if (value == nullptr) return false;
    result = *value;
    return true;
}

bool ProjectDataEngine::getSeat(const std::string& seatId, Seat& result) const
{
    const Seat* value = database.findSeat(seatId);
    if (value == nullptr) return false;
    result = *value;
    return true;
}

bool ProjectDataEngine::getScreening(const std::string& screeningId, Screening& result) const
{
    const Screening* value = database.findScreening(screeningId);
    if (value == nullptr) return false;
    result = *value;
    return true;
}

bool ProjectDataEngine::getTransaction(const std::string& transactionId, SaleTransaction& result) const
{
    const SaleTransaction* value = database.findTransaction(transactionId);
    if (value == nullptr) return false;
    result = *value;
    return true;
}

bool ProjectDataEngine::getTicket(const std::string& ticketId, Ticket& result) const
{
    const Ticket* value = database.findTicket(ticketId);
    if (value == nullptr) return false;
    result = *value;
    return true;
}

bool ProjectDataEngine::addUser(const User& user, std::string& error)
{
    return database.addUser(user, error);
}

bool ProjectDataEngine::addFilm(const Film& film, std::string& error)
{
    return database.addFilm(film, error);
}

bool ProjectDataEngine::addAuditorium(const Auditorium& auditorium, std::string& error)
{
    return database.addAuditorium(auditorium, error);
}

bool ProjectDataEngine::addSeat(const Seat& seat, std::string& error)
{
    return database.addSeat(seat, error);
}

bool ProjectDataEngine::addScreening(const Screening& screening, std::string& error)
{
    return database.addScreening(screening, error);
}

bool ProjectDataEngine::saveUser(const User& user, std::string& error)
{
    return database.saveUser(user, error);
}

bool ProjectDataEngine::saveFilm(const Film& film, std::string& error)
{
    return database.saveFilm(film, error);
}

bool ProjectDataEngine::saveAuditorium(const Auditorium& auditorium, const DoublyLinkedList<Seat>& seats, std::string& error)
{
    return database.saveAuditorium(auditorium, seats, error);
}

bool ProjectDataEngine::saveScreening(const Screening& screening, std::string& error)
{
    return database.saveScreening(screening, error);
}

bool ProjectDataEngine::commitSale(const SaleTransaction& transaction,
                                   const DoublyLinkedList<Ticket>& saleTickets,
                                   std::string& error)
{
    return database.commitSale(transaction, saleTickets, error);
}

Matrix2D ProjectDataEngine::seatMapForScreening(const std::string& screeningId) const
{
    return database.seatMapForScreening(screeningId);
}
}
