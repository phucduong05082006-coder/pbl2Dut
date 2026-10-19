#pragma once

#include "entities/EntityValidation.h"

#include <cstdint>
#include <string>
#include <utility>

class Ticket {
public:
    Ticket(EntityId id, EntityId screeningId, EntityId seatId,
           std::string soldAt, std::int64_t unitPrice)
        : id_(entity_detail::positive(id, "Ticket ID")),
          screeningId_(entity_detail::positive(screeningId, "Screening ID")),
          seatId_(entity_detail::positive(seatId, "Seat ID")),
          soldAt_(entity_detail::dateTime(std::move(soldAt))),
          unitPrice_(entity_detail::positive(unitPrice, "Unit price")) {
    }

    EntityId id() const noexcept { return id_; }
    EntityId screeningId() const noexcept { return screeningId_; }
    EntityId seatId() const noexcept { return seatId_; }
    const std::string& soldAt() const noexcept { return soldAt_; }
    std::int64_t unitPrice() const noexcept { return unitPrice_; }

private:
    EntityId id_;
    EntityId screeningId_;
    EntityId seatId_;
    std::string soldAt_;
    std::int64_t unitPrice_;
};
