#include "ScreeningService.h"
#include "ServiceSupport.h"
#include "../util/DateTime.h"

#include <utility>

namespace cinema
{
using namespace detail;

ScreeningService::ScreeningService(ProjectDataEngine& engine, AuthService& authentication, ServiceClock serviceClock)
    : dataEngine(engine), auth(authentication), clock(std::move(serviceClock)) {}

bool ScreeningService::create(const CreateScreeningRequest& request, Screening& screening, std::string& error)
{
    screening = {};
    if (!auth.requireManager(error)) return false;
    Film film;
    Auditorium room;
    if (!dataEngine.getFilm(request.filmId, film) || film.status != RecordStatus::Active ||
        !dataEngine.getAuditorium(request.auditoriumId, room) || room.status != RecordStatus::Active)
    { error = "Phim/phong khong ton tai hoac khong hoat dong."; return false; }
    std::string now, end, available;
    if (!readTime(clock, now, error)) return false;
    if (request.ticketPrice <= 0 || !calculateTimes(request.startTime, film.durationMinutes, end, available) || request.startTime <= now)
    { error = "Gia ve hoac thoi gian suat chieu khong hop le."; return false; }
    Screening existing;
    std::string id = request.screeningId;
    if (id.empty())
        do { id = newIdentifier("SC"); } while (dataEngine.getScreening(id, existing));
    else if (dataEngine.getScreening(id, existing))
    { error = "Ma suat chieu da ton tai."; return false; }
    QuerySpec<Seat> seats;
    seats.where = [&](const Seat& seat) { return seat.auditoriumId == room.auditoriumId && seat.status == RecordStatus::Active; };
    if (dataEngine.query(seats).isEmpty()) { error = "Phong khong co ghe hoat dong."; return false; }
    const Screening candidate{id, request.filmId, request.auditoriumId, request.startTime, end, available,
                              request.ticketPrice, request.openForSale ? ScreeningStatus::Open : ScreeningStatus::Draft};
    if (!dataEngine.saveScreening(candidate, error)) return false;
    screening = candidate;
    return true;
}

bool ScreeningService::changeStatus(const std::string& screeningId, ScreeningStatus status, std::string& error)
{
    if (!auth.requireManager(error)) return false;
    Screening screening;
    if (!dataEngine.getScreening(screeningId, screening)) { error = "Khong tim thay suat chieu."; return false; }
    std::string now;
    if (!readTime(clock, now, error)) return false;
    if (screening.status == ScreeningStatus::Completed && status != ScreeningStatus::Completed)
    { error = "Khong doi trang thai suat chieu da hoan tat."; return false; }
    if (status == ScreeningStatus::Draft && screening.status != ScreeningStatus::Draft)
    { error = "Khong chuyen suat chieu da mo ban ve nhap."; return false; }
    if (status == ScreeningStatus::Open)
    {
        Film film;
        Auditorium room;
        if (now >= screening.startTime || !dataEngine.getFilm(screening.filmId, film) || film.status != RecordStatus::Active ||
            !dataEngine.getAuditorium(screening.auditoriumId, room) || room.status != RecordStatus::Active)
        { error = "Khong the mo ban suat chieu nay."; return false; }
    }
    if (status == ScreeningStatus::Completed && now < screening.endTime)
    { error = "Suat chieu chua ket thuc."; return false; }
    screening.status = status;
    return dataEngine.saveScreening(screening, error);
}

DoublyLinkedList<Screening> ScreeningService::openScreenings() const { return openScreeningsFor({}, {}); }

DoublyLinkedList<ScreeningDetails> ScreeningService::list(const std::string& filmId, const std::string& roomId,
    const std::string& date, std::optional<ScreeningStatus> status, bool saleOnly) const
{
    requireRead(auth);
    std::time_t epoch;
    if (!date.empty() && !parseTimestamp(date + " 00:00:00", epoch)) throw std::invalid_argument("Ngay tra cuu khong hop le.");
    if (status && *status != ScreeningStatus::Draft && *status != ScreeningStatus::Open &&
        *status != ScreeningStatus::Closed && *status != ScreeningStatus::Completed)
        throw std::invalid_argument("Trang thai suat chieu khong hop le.");
    std::string now, error;
    if (!readTime(clock, now, error)) throw std::runtime_error(error);
    QuerySpec<Screening> specification;
    specification.where = [&](const Screening& show) {
        return (filmId.empty() || show.filmId == filmId) && (roomId.empty() || show.auditoriumId == roomId) &&
               (date.empty() || show.startTime.substr(0, 10) == date) && (!status || show.status == *status) &&
               (!saleOnly || (show.status == ScreeningStatus::Open && show.startTime > now));
    };
    specification.orderBy = [](const Screening& left, const Screening& right) { return left.startTime < right.startTime; };
    const auto records = dataEngine.query(specification);
    DoublyLinkedList<ScreeningDetails> result;
    for (const Node<Screening>* node = records.getHead(); node != nullptr; node = node->next)
    {
        ScreeningDetails details;
        details.screening = node->data;
        if (!dataEngine.getFilm(node->data.filmId, details.film) || !dataEngine.getAuditorium(node->data.auditoriumId, details.auditorium))
            throw std::runtime_error("Suat chieu co tham chieu khong hop le.");
        if (!saleOnly || (details.film.status == RecordStatus::Active && details.auditorium.status == RecordStatus::Active))
            result.addLast(details);
    }
    return result;
}

DoublyLinkedList<Screening> ScreeningService::openScreeningsFor(const std::string& filmId, const std::string& roomId, const std::string& date) const
{
    requireRead(auth);
    std::time_t epoch;
    if (!date.empty() && !parseTimestamp(date + " 00:00:00", epoch)) throw std::invalid_argument("Ngay tra cuu khong hop le.");
    std::string now, error;
    if (!readTime(clock, now, error)) throw std::runtime_error(error);
    QuerySpec<Screening> specification;
    specification.where = [&](const Screening& screening) {
        return screening.status == ScreeningStatus::Open && screening.startTime > now &&
               (filmId.empty() || screening.filmId == filmId) && (roomId.empty() || screening.auditoriumId == roomId) &&
               (date.empty() || screening.startTime.substr(0, 10) == date);
    };
    specification.orderBy = [](const Screening& left, const Screening& right) { return left.startTime < right.startTime; };
    return dataEngine.query(specification);
}
}
