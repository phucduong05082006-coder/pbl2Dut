#pragma once

#include "entities/EntityValidation.h"

#include <cstdint>
#include <string>
#include <utility>

class Screening {
public:
    Screening(EntityId id, EntityId filmId, EntityId auditoriumId,
              std::string startTime, std::int64_t ticketPrice)
        : id_(entity_detail::positive(id, "Screening ID")),
          filmId_(entity_detail::positive(filmId, "Film ID")),
          auditoriumId_(entity_detail::positive(auditoriumId, "Auditorium ID")),
          startTime_(entity_detail::dateTime(std::move(startTime))),
          ticketPrice_(entity_detail::positive(ticketPrice, "Ticket price")) {
    }

    EntityId id() const noexcept { return id_; }
    EntityId filmId() const noexcept { return filmId_; }
    EntityId auditoriumId() const noexcept { return auditoriumId_; }
    const std::string& startTime() const noexcept { return startTime_; }
    std::int64_t ticketPrice() const noexcept { return ticketPrice_; }

private:
    EntityId id_;
    EntityId filmId_;
    EntityId auditoriumId_;
    std::string startTime_;
    std::int64_t ticketPrice_;
};
