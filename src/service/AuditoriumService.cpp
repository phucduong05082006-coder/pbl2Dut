#include "AuditoriumService.h"
#include "ServiceSupport.h"

#include <limits>

namespace cinema
{
using namespace detail;

AuditoriumService::AuditoriumService(ProjectDataEngine& engine, AuthService& authentication) : dataEngine(engine), auth(authentication) {}

bool AuditoriumService::save(const Auditorium& auditorium, const DoublyLinkedList<Seat>& seats, std::string& error)
{
    if (!auth.requireManager(error)) return false;
    Auditorium existing;
    if (dataEngine.getAuditorium(auditorium.auditoriumId, existing))
    {
        if (auditorium.status == RecordStatus::Inactive && hasFutureBookings(dataEngine, {}, auditorium.auditoriumId))
        { error = "Phong con lich chieu chua ket thuc."; return false; }
        bool layoutChanged = existing.rows != auditorium.rows || existing.columns != auditorium.columns ||
                             existing.capacity != auditorium.capacity;
        const auto previousSeats = seatsFor(auditorium.auditoriumId);
        layoutChanged = layoutChanged || previousSeats.size() != seats.size();
        for (const Node<Seat>* node = seats.getHead(); node != nullptr; node = node->next)
        {
            Seat previous;
            if (!dataEngine.getSeat(node->data.seatId, previous) || previous.auditoriumId != auditorium.auditoriumId ||
                previous.seatCode != node->data.seatCode || previous.rowLabel != node->data.rowLabel ||
                previous.seatNumber != node->data.seatNumber || previous.status != node->data.status || previous.type != node->data.type)
                layoutChanged = true;
        }
        if (layoutChanged && hasBookings(dataEngine, {}, auditorium.auditoriumId))
        { error = "Khong sua so do ghe cua phong da co lich chieu."; return false; }
    }
    return dataEngine.saveAuditorium(auditorium, seats, error);
}

bool AuditoriumService::find(const std::string& auditoriumId, Auditorium& auditorium) const
{
    auditorium = {};
    std::string error;
    return auth.requireLogin(error) && dataEngine.getAuditorium(auditoriumId, auditorium);
}

bool AuditoriumService::saveGrid(Auditorium auditorium, bool creating, std::string& error)
{
    if (!auth.requireManager(error)) return false;
    Auditorium existing;
    const bool found = dataEngine.getAuditorium(auditorium.auditoriumId, existing);
    if (creating == found)
    { error = creating ? "Ma phong da ton tai." : "Khong tim thay phong."; return false; }
    if (auditorium.rows < 1 || auditorium.rows > 26 || auditorium.columns < 1 ||
        static_cast<long long>(auditorium.rows) * auditorium.columns > std::numeric_limits<int>::max())
    { error = "So hang/cot phong khong hop le."; return false; }
    auditorium.capacity = auditorium.rows * auditorium.columns;
    DoublyLinkedList<Seat> seats;
    if (found && auditorium.rows == existing.rows && auditorium.columns == existing.columns)
        seats = seatsFor(auditorium.auditoriumId);
    else
    {
        for (int row = 0; row < auditorium.rows; ++row)
            for (int column = 1; column <= auditorium.columns; ++column)
            {
                const std::string label(1, static_cast<char>('A' + row));
                const std::string code = label + (column < 10 ? "0" : "") + std::to_string(column);
                seats.addLast({auditorium.auditoriumId + "_" + code, auditorium.auditoriumId, code,
                               label, column, SeatType::Standard, RecordStatus::Active});
            }
    }
    return save(auditorium, seats, error);
}

DoublyLinkedList<Auditorium> AuditoriumService::list(std::optional<RecordStatus> status) const
{
    requireRead(auth);
    if (status && *status != RecordStatus::Active && *status != RecordStatus::Inactive)
        throw std::invalid_argument("Trang thai phong khong hop le.");
    QuerySpec<Auditorium> specification;
    specification.where = [status](const Auditorium& room) { return !status || room.status == *status; };
    specification.orderBy = [](const Auditorium& left, const Auditorium& right) {
        return left.auditoriumId < right.auditoriumId;
    };
    return dataEngine.query(specification);
}

DoublyLinkedList<Auditorium> AuditoriumService::activeAuditoriums() const
{
    return list(RecordStatus::Active);
}

DoublyLinkedList<Seat> AuditoriumService::seatsFor(const std::string& auditoriumId) const
{
    requireRead(auth);
    QuerySpec<Seat> specification;
    specification.where = [&](const Seat& seat) { return seat.auditoriumId == auditoriumId; };
    return dataEngine.query(specification);
}
}
