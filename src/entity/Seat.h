#ifndef CINEMA_ENTITY_SEAT_H
#define CINEMA_ENTITY_SEAT_H

#include <string>

#include "../enums/DomainEnums.h"

namespace cinema
{
struct Seat
{
    std::string seatId;
    std::string auditoriumId;
    std::string seatCode;
    std::string rowLabel;
    int seatNumber = 0;
    SeatType type = SeatType::Standard;
    RecordStatus status = RecordStatus::Active;

    bool operator==(const Seat& other) const { return seatId == other.seatId; }
};
}

#endif
