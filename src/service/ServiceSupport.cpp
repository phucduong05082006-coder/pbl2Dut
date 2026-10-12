#include "ServiceSupport.h"
#include "AuthService.h"
#include "../util/DateTime.h"

#include <chrono>
#include <ctime>
#include <stdexcept>

namespace cinema::detail
{
std::string currentTime(const ServiceClock& clock)
{
    if (clock) return clock();
    const std::time_t now = std::time(nullptr);
    const std::tm* calendar = std::localtime(&now);
    if (calendar == nullptr) throw std::runtime_error("Khong doc duoc dong ho he thong.");
    return formatTimestamp(*calendar);
}

bool readTime(const ServiceClock& clock, std::string& text, std::string& error)
{
    text = currentTime(clock);
    std::time_t epoch;
    if (parseTimestamp(text, epoch)) return true;
    error = "Dong ho khong tra ve thoi gian hop le.";
    return false;
}

void requireRead(const AuthService& auth)
{
    std::string error;
    if (!auth.requireLogin(error)) throw std::runtime_error(error);
}

std::string newIdentifier(const char* prefix)
{
    return std::string(prefix) + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
}

bool hasBookings(ProjectDataEngine& engine, const std::string& filmId, const std::string& roomId)
{
    QuerySpec<Screening> specification;
    specification.where = [&](const Screening& screening) {
        return (!filmId.empty() && screening.filmId == filmId) ||
               (!roomId.empty() && screening.auditoriumId == roomId);
    };
    specification.limit = 1;
    return !engine.query(specification).isEmpty();
}

bool hasFutureBookings(ProjectDataEngine& engine, const std::string& filmId, const std::string& roomId)
{
    const std::string now = currentTime({});
    QuerySpec<Screening> specification;
    specification.where = [&](const Screening& screening) {
        return screening.status != ScreeningStatus::Completed && screening.roomAvailableUntil > now &&
               ((!filmId.empty() && screening.filmId == filmId) || (!roomId.empty() && screening.auditoriumId == roomId));
    };
    specification.limit = 1;
    return !engine.query(specification).isEmpty();
}
}
