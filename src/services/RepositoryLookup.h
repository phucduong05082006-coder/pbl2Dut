#pragma once

#include "entities/EntityValidation.h"

#include <stdexcept>

namespace service_detail {

template <typename RepositoryType>
const auto& require(const RepositoryType& repository, EntityId id) {
    entity_detail::positive(id, "ID");
    const auto* record = repository.findById(id);
    if (record == nullptr) {
        throw std::out_of_range("Record not found");
    }
    return *record;
}

}
