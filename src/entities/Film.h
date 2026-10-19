#pragma once

#include "entities/EntityValidation.h"

#include <cstdint>
#include <string>
#include <utility>

class Film {
public:
    Film(EntityId id, std::string title, std::int64_t durationMinutes)
        : id_(entity_detail::positive(id, "Film ID")),
          title_(entity_detail::name(std::move(title))),
          durationMinutes_(entity_detail::positive(durationMinutes, "Duration")) {
    }

    EntityId id() const noexcept { return id_; }
    const std::string& title() const noexcept { return title_; }
    std::int64_t durationMinutes() const noexcept { return durationMinutes_; }

private:
    EntityId id_;
    std::string title_;
    std::int64_t durationMinutes_;
};
