#ifndef CINEMA_SERVICE_AUDITORIUM_SERVICE_H
#define CINEMA_SERVICE_AUDITORIUM_SERVICE_H

#include <optional>
#include "AuthService.h"
#include "../entity/Auditorium.h"
#include "../entity/Seat.h"

namespace cinema
{
class AuditoriumService final
{
public:
    AuditoriumService(ProjectDataEngine& dataEngine, AuthService& auth);

    bool save(const Auditorium& auditorium,
              const DoublyLinkedList<Seat>& seats,
              std::string& error);
    bool saveGrid(Auditorium auditorium, bool creating, std::string& error);
    bool find(const std::string& auditoriumId, Auditorium& auditorium) const;
    DoublyLinkedList<Auditorium> list(std::optional<RecordStatus> status = std::nullopt) const;
    DoublyLinkedList<Auditorium> activeAuditoriums() const;
    DoublyLinkedList<Seat> seatsFor(const std::string& auditoriumId) const;

private:
    ProjectDataEngine& dataEngine;
    AuthService& auth;
};
}

#endif
