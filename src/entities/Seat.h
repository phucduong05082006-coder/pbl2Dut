#pragma once

#include "entities/EntityValidation.h"

#include <cstdint>

class Seat {
public:
    Seat(EntityId id, EntityId auditoriumId, std::int64_t rowNumber, std::int64_t seatNumber)
        : id_(entity_detail::positive(id, "Seat ID")),
          auditoriumId_(entity_detail::positive(auditoriumId, "Auditorium ID")),
          rowNumber_(entity_detail::positive(rowNumber, "Row number")),
          seatNumber_(entity_detail::positive(seatNumber, "Seat number")) {
    }

    EntityId id() const noexcept { return id_; }
    EntityId auditoriumId() const noexcept { return auditoriumId_; }
    std::int64_t rowNumber() const noexcept { return rowNumber_; }
    std::int64_t seatNumber() const noexcept { return seatNumber_; }

private:
    EntityId id_;
    EntityId auditoriumId_;
    std::int64_t rowNumber_;
    std::int64_t seatNumber_;
};
