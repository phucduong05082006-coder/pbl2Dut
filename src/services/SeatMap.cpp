#include "services/SeatMap.h"

#include <cstdint>
#include <limits>
#include <stdexcept>

std::unique_ptr<SeatMap> makeSeatMap(const Auditorium& room, const SeatRepository& seats) {
    const auto maximum = std::numeric_limits<std::size_t>::max();
    if (static_cast<std::uint64_t>(room.rowCount()) > maximum
        || static_cast<std::uint64_t>(room.seatsPerRow()) > maximum) {
        throw std::length_error("Room dimensions exceed address space");
    }
    auto result = std::make_unique<SeatMap>(
        static_cast<std::size_t>(room.rowCount()), static_cast<std::size_t>(room.seatsPerRow()));
    std::size_t count = 0;
    seats.forEach([&](const Seat& seat) {
        if (seat.auditoriumId() != room.id()) {
            return;
        }
        if (seat.rowNumber() > room.rowCount() || seat.seatNumber() > room.seatsPerRow()) {
            throw std::invalid_argument("Seat outside room");
        }
        auto& cell = result->at(static_cast<std::size_t>(seat.rowNumber() - 1),
                                static_cast<std::size_t>(seat.seatNumber() - 1));
        if (cell.seatId != 0) {
            throw std::invalid_argument("Duplicate seat position");
        }
        cell.seatId = seat.id();
        ++count;
    });
    if (count != result->size()) {
        throw std::invalid_argument("Incomplete room seats");
    }
    return result;
}
