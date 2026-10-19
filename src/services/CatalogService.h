#pragma once

#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/SeatRepository.h"
#include "services/SeatMap.h"

#include <cstdint>
#include <memory>
#include <string>

class CatalogService {
public:
    CatalogService(FilmRepository& films, AuditoriumRepository& rooms,
                   SeatRepository& seats, const ScreeningRepository& screenings)
        : films_(films), rooms_(rooms), seats_(seats), screenings_(screenings) {}

    EntityId addFilm(std::string title, std::int64_t duration);
    void updateFilm(EntityId id, std::string title, std::int64_t duration);
    void eraseFilm(EntityId id);
    const Film& film(EntityId id) const;

    template <typename Function>
    void films(const std::string& titlePart, Function visit) const {
        films_.forEach([&](const Film& item) {
            if (item.title().find(titlePart) != std::string::npos) {
                visit(item);
            }
        });
    }

    EntityId addRoom(std::string name, std::int64_t rows, std::int64_t columns);
    void updateRoom(EntityId id, std::string name, std::int64_t rows, std::int64_t columns);
    void eraseRoom(EntityId id);
    const Auditorium& room(EntityId id) const;

    template <typename Function>
    void rooms(Function visit) const {
        rooms_.forEach(visit);
    }

    std::unique_ptr<SeatMap> seatMap(EntityId roomId) const;

private:
    bool filmReferenced(EntityId id) const;
    void requireRoomUnused(EntityId id) const;
    void prepareSeats(SeatRepository& staged, EntityId roomId,
                      const Auditorium* replacement) const;

    FilmRepository& films_;
    AuditoriumRepository& rooms_;
    SeatRepository& seats_;
    const ScreeningRepository& screenings_;
};
