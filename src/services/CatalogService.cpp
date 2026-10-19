#include "services/CatalogService.h"

#include "services/RepositoryLookup.h"

#include <limits>
#include <stdexcept>
#include <utility>

EntityId CatalogService::addFilm(std::string title, std::int64_t duration) {
    const Film candidate(films_.nextId(), std::move(title), duration);
    films_.insert(candidate);
    return candidate.id();
}

bool CatalogService::filmReferenced(EntityId id) const {
    bool referenced = false;
    screenings_.forEach([&](const Screening& item) { referenced |= item.filmId() == id; });
    return referenced;
}

const Film& CatalogService::film(EntityId id) const {
    return service_detail::require(films_, id);
}

const Auditorium& CatalogService::room(EntityId id) const {
    return service_detail::require(rooms_, id);
}

void CatalogService::updateFilm(EntityId id, std::string title, std::int64_t duration) {
    const Film candidate(id, std::move(title), duration);
    if (film(id).durationMinutes() != duration && filmReferenced(id)) {
        throw std::invalid_argument("Cannot change duration of a scheduled film");
    }
    films_.update(id, candidate);
}

void CatalogService::eraseFilm(EntityId id) {
    film(id);
    if (filmReferenced(id)) {
        throw std::invalid_argument("Film is referenced by screenings");
    }
    films_.erase(id);
}

void CatalogService::requireRoomUnused(EntityId id) const {
    room(id);
    screenings_.forEach([&](const Screening& item) {
        if (item.auditoriumId() == id) {
            throw std::invalid_argument("Room is referenced by screenings");
        }
    });
}

void CatalogService::prepareSeats(SeatRepository& staged, EntityId roomId,
                                  const Auditorium* replacement) const {
    // ponytail: rebuild all seats for atomic room edits; use batch updates if data outgrows RAM.
    seats_.forEach([&](const Seat& seat) {
        if (seat.auditoriumId() != roomId) {
            staged.insert(seat);
        }
    });
    if (replacement == nullptr) {
        return;
    }
    const EntityId first = seats_.nextId();
    if (replacement->capacity() - 1 > std::numeric_limits<EntityId>::max() - first) {
        throw std::overflow_error("Seat IDs exhausted");
    }
    for (std::int64_t offset = 0; offset < replacement->capacity(); ++offset) {
        staged.insert(Seat(first + offset, roomId,
                           offset / replacement->seatsPerRow() + 1,
                           offset % replacement->seatsPerRow() + 1));
    }
}

EntityId CatalogService::addRoom(std::string name, std::int64_t rows, std::int64_t columns) {
    const Auditorium candidate(rooms_.nextId(), std::move(name), rows, columns);
    SeatRepository staged;
    prepareSeats(staged, candidate.id(), &candidate);
    rooms_.insert(candidate);
    seats_.replaceWith(staged);
    return candidate.id();
}

void CatalogService::updateRoom(EntityId id, std::string name,
                                std::int64_t rows, std::int64_t columns) {
    requireRoomUnused(id);
    const Auditorium candidate(id, std::move(name), rows, columns);
    SeatRepository staged;
    prepareSeats(staged, id, &candidate);
    rooms_.update(id, candidate);
    seats_.replaceWith(staged);
}

void CatalogService::eraseRoom(EntityId id) {
    requireRoomUnused(id);
    SeatRepository staged;
    prepareSeats(staged, id, nullptr);
    rooms_.erase(id);
    seats_.replaceWith(staged);
}

std::unique_ptr<SeatMap> CatalogService::seatMap(EntityId roomId) const {
    return makeSeatMap(room(roomId), seats_);
}
