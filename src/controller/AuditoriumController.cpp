#include "AuditoriumController.h"

#include <exception>

namespace cinema
{
AuditoriumController::AuditoriumController(AuditoriumService& service) : auditoriums(service) {}

bool AuditoriumController::saveGrid(const Auditorium& room, bool creating, std::string& error)
{
    error.clear();
    try { return auditoriums.saveGrid(room, creating, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool AuditoriumController::list(std::optional<RecordStatus> status,
                                DoublyLinkedList<Auditorium>& result, std::string& error) const
{
    error.clear();
    result.clear();
    try { result = auditoriums.list(status); return true; }
    catch (const std::exception& exception) { result.clear(); error = exception.what(); return false; }
}

bool AuditoriumController::details(const std::string& auditoriumId, Auditorium& room,
                                   DoublyLinkedList<Seat>& seats, std::string& error) const
{
    error.clear();
    room = {};
    seats.clear();
    try
    {
        if (!auditoriums.find(auditoriumId, room))
        {
            error = "Khong tim thay phong hoac phien dang nhap khong con hop le.";
            return false;
        }
        seats = auditoriums.seatsFor(auditoriumId);
        if (room.rows < 1 || room.rows > 26 || room.columns < 1 ||
            static_cast<long long>(room.rows) * room.columns != room.capacity || seats.size() != room.capacity)
        {
            room = {};
            seats.clear();
            error = "Kich thuoc phong hoac so luong ghe trong du lieu TXT khong hop le.";
            return false;
        }
        return true;
    }
    catch (const std::exception& exception)
    {
        room = {};
        seats.clear();
        error = exception.what();
        return false;
    }
}
}
