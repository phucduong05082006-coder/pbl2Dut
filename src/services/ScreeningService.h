#pragma once

#include "entities/EntityValidation.h"
#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/TicketRepository.h"
#include "services/ServiceClock.h"

#include <cstdint>
#include <string>

class ScreeningService {
public:
    ScreeningService(const FilmRepository& films, const AuditoriumRepository& rooms,
                     ScreeningRepository& screenings, const TicketRepository& tickets,
                     ServiceClock clock = localMinuteNow);

    EntityId add(EntityId filmId, EntityId roomId, std::string startTime, std::int64_t price);
    void update(EntityId id, EntityId filmId, EntityId roomId,
                std::string startTime, std::int64_t price);
    void erase(EntityId id);
    const Screening& find(EntityId id) const;
    std::string endTime(EntityId id) const;

    // Empty date and zero IDs mean no filter; callbacks receive read-only records.
    template <typename Function>
    void list(const std::string& date, EntityId filmId, EntityId roomId,
              bool futureOnly, Function visit) const {
        validateFilters(date, filmId, roomId);
        const std::string now = futureOnly ? entity_detail::dateTime(clock_()) : "";
        screenings_.forEach([&](const Screening& item) {
            if ((date.empty() || item.startTime().compare(0, 10, date) == 0)
                && (filmId == 0 || item.filmId() == filmId)
                && (roomId == 0 || item.auditoriumId() == roomId)
                && (!futureOnly || item.startTime() > now)) {
                visit(item);
            }
        });
    }

private:
    void validate(const Screening& candidate) const;
    void requireNoTickets(EntityId id) const;
    static void validateFilters(const std::string& date, EntityId filmId, EntityId roomId);

    const FilmRepository& films_;
    const AuditoriumRepository& rooms_;
    ScreeningRepository& screenings_;
    const TicketRepository& tickets_;
    ServiceClock clock_;
};
