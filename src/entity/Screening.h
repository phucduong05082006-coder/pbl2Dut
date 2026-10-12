#ifndef CINEMA_ENTITY_SCREENING_H
#define CINEMA_ENTITY_SCREENING_H

#include <string>

#include "../enums/DomainEnums.h"

namespace cinema
{
struct Screening
{
    std::string screeningId;
    std::string filmId;
    std::string auditoriumId;
    std::string startTime;
    std::string endTime;
    std::string roomAvailableUntil;
    long long ticketPrice = 0;
    ScreeningStatus status = ScreeningStatus::Draft;

    bool operator==(const Screening& other) const { return screeningId == other.screeningId; }
};

inline int compareScreeningByStart(const Screening& left, const Screening& right)
{
    if (left.startTime < right.startTime) return -1;
    if (left.startTime > right.startTime) return 1;
    if (left.screeningId < right.screeningId) return -1;
    if (left.screeningId > right.screeningId) return 1;
    return 0;
}
}

#endif
