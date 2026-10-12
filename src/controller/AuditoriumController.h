#ifndef CINEMA_CONTROLLER_AUDITORIUM_CONTROLLER_H
#define CINEMA_CONTROLLER_AUDITORIUM_CONTROLLER_H

#include "../service/AuditoriumService.h"

namespace cinema
{
class AuditoriumController final
{
public:
    explicit AuditoriumController(AuditoriumService& service);
    bool list(std::optional<RecordStatus> status, DoublyLinkedList<Auditorium>& result,
              std::string& error) const;
    bool details(const std::string& auditoriumId, Auditorium& room,
                 DoublyLinkedList<Seat>& seats, std::string& error) const;
    bool saveGrid(const Auditorium& room, bool creating, std::string& error);

private:
    AuditoriumService& auditoriums;
};
}

#endif
