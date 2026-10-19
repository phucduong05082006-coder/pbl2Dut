#include "services/ScreeningService.h"

#include "services/RepositoryLookup.h"
#include "services/ScreeningTime.h"
#include "services/ScreeningValidation.h"

#include <stdexcept>
#include <utility>

ScreeningService::ScreeningService(const FilmRepository& films,
                                   const AuditoriumRepository& rooms,
                                   ScreeningRepository& screenings,
                                   const TicketRepository& tickets, ServiceClock clock)
    : films_(films), rooms_(rooms), screenings_(screenings), tickets_(tickets), clock_(clock) {
    if (clock_ == nullptr) {
        throw std::invalid_argument("Clock is required");
    }
}

const Screening& ScreeningService::find(EntityId id) const {
    return service_detail::require(screenings_, id);
}

std::string ScreeningService::endTime(EntityId id) const {
    return screening_time::formatMinute(screening_time::endMinute(find(id), films_));
}

void ScreeningService::validate(const Screening& candidate) const {
    service_validation::screening(candidate, films_, rooms_, screenings_);
    if (screening_time::minutes(candidate.startTime()) <= screening_time::minutes(clock_())) {
        throw std::invalid_argument("Screening must start in the future");
    }
}

EntityId ScreeningService::add(EntityId filmId, EntityId roomId,
                               std::string startTime, std::int64_t price) {
    const Screening candidate(screenings_.nextId(), filmId, roomId, std::move(startTime), price);
    validate(candidate);
    screenings_.insert(candidate);
    return candidate.id();
}

void ScreeningService::requireNoTickets(EntityId id) const {
    find(id);
    tickets_.forEach([&](const Ticket& ticket) {
        if (ticket.screeningId() == id) {
            throw std::invalid_argument("Screening already has tickets");
        }
    });
}

void ScreeningService::update(EntityId id, EntityId filmId, EntityId roomId,
                              std::string startTime, std::int64_t price) {
    requireNoTickets(id);
    const Screening candidate(id, filmId, roomId, std::move(startTime), price);
    validate(candidate);
    screenings_.update(id, candidate);
}

void ScreeningService::erase(EntityId id) {
    requireNoTickets(id);
    screenings_.erase(id);
}

void ScreeningService::validateFilters(const std::string& date,
                                       EntityId filmId, EntityId roomId) {
    if (!date.empty()) {
        screening_time::validateDate(date);
    }
    if (filmId < 0 || roomId < 0) {
        throw std::invalid_argument("Filter IDs must not be negative");
    }
}
