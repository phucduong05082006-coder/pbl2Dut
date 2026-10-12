#ifndef CINEMA_ENTITY_AUDITORIUM_H
#define CINEMA_ENTITY_AUDITORIUM_H

#include <string>

#include "../enums/DomainEnums.h"

namespace cinema
{
struct Auditorium
{
    std::string auditoriumId;
    std::string name;
    int rows = 0;
    int columns = 0;
    int capacity = 0;
    RecordStatus status = RecordStatus::Active;

    bool operator==(const Auditorium& other) const { return auditoriumId == other.auditoriumId; }
};
}

#endif
