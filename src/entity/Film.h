#ifndef CINEMA_ENTITY_FILM_H
#define CINEMA_ENTITY_FILM_H

#include <string>

#include "../enums/DomainEnums.h"

namespace cinema
{
struct Film
{
    std::string filmId;
    std::string title;
    int durationMinutes = 0;
    std::string ageRating;
    RecordStatus status = RecordStatus::Active;

    bool operator==(const Film& other) const { return filmId == other.filmId; }
};
}

#endif
