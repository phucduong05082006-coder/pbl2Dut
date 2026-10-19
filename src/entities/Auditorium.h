#pragma once

#include "entities/EntityValidation.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

class Auditorium {
public:
    Auditorium(EntityId id, std::string name, std::int64_t rowCount, std::int64_t seatsPerRow)
        : id_(entity_detail::positive(id, "Auditorium ID")),
          name_(entity_detail::name(std::move(name))),
          rowCount_(entity_detail::positive(rowCount, "Row count")),
          seatsPerRow_(entity_detail::positive(seatsPerRow, "Seats per row")) {
        if (rowCount_ > std::numeric_limits<std::int64_t>::max() / seatsPerRow_) {
            throw std::overflow_error("Auditorium capacity overflow");
        }
    }

    EntityId id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    std::int64_t rowCount() const noexcept { return rowCount_; }
    std::int64_t seatsPerRow() const noexcept { return seatsPerRow_; }
    std::int64_t capacity() const noexcept { return rowCount_ * seatsPerRow_; }

private:
    EntityId id_;
    std::string name_;
    std::int64_t rowCount_;
    std::int64_t seatsPerRow_;
};
