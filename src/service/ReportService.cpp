#include "ReportService.h"
#include "ServiceSupport.h"
#include "../util/DateTime.h"

#include <limits>

namespace cinema
{
using namespace detail;

ReportService::ReportService(ProjectDataEngine& engine, AuthService& authentication) : dataEngine(engine), auth(authentication) {}

bool ReportService::revenueByFilm(const RevenueQuery& query, RevenueReport& report, std::string& error) const
{
    report = {};
    if (!auth.requireManager(error)) return false;
    std::time_t epoch;
    if ((!query.fromCreatedAt.empty() && !parseTimestamp(query.fromCreatedAt, epoch)) ||
        (!query.toCreatedAt.empty() && !parseTimestamp(query.toCreatedAt, epoch)) ||
        (!query.fromCreatedAt.empty() && !query.toCreatedAt.empty() && query.fromCreatedAt > query.toCreatedAt))
    { error = "Khoang thoi gian bao cao khong hop le."; return false; }
    AggregateSpec<Ticket> specification;
    specification.where = [&](const Ticket& ticket) {
        SaleTransaction transaction;
        Screening screening;
        return ticket.status == TicketStatus::Issued && dataEngine.getTransaction(ticket.transactionId, transaction) &&
               transaction.status == TransactionStatus::Completed && transaction.paymentStatus == PaymentStatus::Confirmed &&
               (query.fromCreatedAt.empty() || transaction.createdAt >= query.fromCreatedAt) &&
               (query.toCreatedAt.empty() || transaction.createdAt <= query.toCreatedAt) &&
               dataEngine.getScreening(ticket.screeningId, screening) &&
               (query.filmId.empty() || screening.filmId == query.filmId) &&
               (query.screeningId.empty() || ticket.screeningId == query.screeningId);
    };
    specification.groupBy = [&](const Ticket& ticket) {
        Screening screening;
        if (!dataEngine.getScreening(ticket.screeningId, screening)) throw std::runtime_error("Du lieu bao cao khong nhat quan.");
        return screening.filmId;
    };
    specification.value = [](const Ticket& ticket) { return ticket.unitPrice; };
    RevenueReport candidate;
    try
    {
        const auto grouped = dataEngine.aggregate(specification);
        for (const Node<AggregateRow>* node = grouped.getHead(); node != nullptr; node = node->next)
        {
            Film film;
            if (!dataEngine.getFilm(node->data.key, film) || node->data.total < 0 ||
                candidate.revenue > std::numeric_limits<long long>::max() - node->data.total)
            { error = "Du lieu hoac tong tien bao cao khong hop le."; return false; }
            candidate.rows.addLast({film.filmId, film.title, node->data.count, node->data.total});
            candidate.ticketCount += node->data.count;
            candidate.revenue += node->data.total;
        }
    }
    catch (const std::exception& exception) { error = exception.what(); return false; }
    report = candidate;
    return true;
}

bool ReportService::occupancy(const std::string& screeningId, OccupancyReport& report, std::string& error) const
{
    report = {};
    if (!auth.requireManager(error)) return false;
    Screening screening;
    if (!dataEngine.getScreening(screeningId, screening)) { error = "Khong tim thay suat chieu."; return false; }
    Auditorium room;
    if (!dataEngine.getAuditorium(screening.auditoriumId, room) || room.capacity <= 0)
    { error = "Suc chua phong khong hop le."; return false; }
    const int capacity = room.capacity;
    QuerySpec<Ticket> tickets;
    tickets.where = [&](const Ticket& ticket) {
        SaleTransaction transaction;
        return ticket.screeningId == screeningId && ticket.status == TicketStatus::Issued &&
               dataEngine.getTransaction(ticket.transactionId, transaction) &&
               transaction.status == TransactionStatus::Completed && transaction.paymentStatus == PaymentStatus::Confirmed;
    };
    report.soldSeats = dataEngine.query(tickets).size();
    if (report.soldSeats > capacity) { report = {}; error = "So ve vuot suc chua phong."; return false; }
    report.availableSeats = capacity - report.soldSeats;
    report.percent = capacity == 0 ? 0.0 : 100.0 * report.soldSeats / capacity;
    return true;
}
}
