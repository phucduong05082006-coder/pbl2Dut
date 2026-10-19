#include "services/TicketService.h"

#include "entities/EntityValidation.h"
#include "services/RepositoryLookup.h"
#include "services/ScreeningTime.h"

#include <limits>
#include <stdexcept>

TicketService::TicketService(const FilmRepository& films, const AuditoriumRepository& rooms,
                             const SeatRepository& seats,
                             const ScreeningRepository& screenings,
                             TicketRepository& tickets, ServiceClock clock)
    : films_(films), rooms_(rooms), seats_(seats), screenings_(screenings),
      tickets_(tickets), clock_(clock) {
    if (clock_ == nullptr) {
        throw std::invalid_argument("Clock is required");
    }
}

SaleDetails TicketService::details(EntityId screeningId, EntityId seatId) const {
    const auto& screening = service_detail::require(screenings_, screeningId);
    const auto& seat = service_detail::require(seats_, seatId);
    const auto& room = service_detail::require(rooms_, screening.auditoriumId());
    if (seat.auditoriumId() != room.id()
        || seat.rowNumber() > room.rowCount() || seat.seatNumber() > room.seatsPerRow()) {
        throw std::invalid_argument("Seat does not belong to screening room");
    }
    return {service_detail::require(films_, screening.filmId()), room, screening, seat,
            screening_time::formatMinute(screening_time::endMinute(screening, films_))};
}

void TicketService::requireAvailable(const SaleDetails& sale, const std::string& now) const {
    entity_detail::dateTime(now);
    if (now >= sale.screening.startTime()) {
        throw std::invalid_argument("Screening has already started");
    }
    tickets_.forEach([&](const Ticket& ticket) {
        if (ticket.screeningId() == sale.screening.id() && ticket.seatId() == sale.seat.id()) {
            throw std::invalid_argument("Seat is already sold");
        }
    });
}

SaleDetails TicketService::preview(EntityId screeningId, EntityId seatId) const {
    auto sale = details(screeningId, seatId);
    requireAvailable(sale, clock_());
    return sale;
}

EntityId TicketService::sell(EntityId screeningId, EntityId seatId, bool cashConfirmed) {
    if (!cashConfirmed) {
        throw std::invalid_argument("Cash confirmation is required");
    }
    const auto sale = details(screeningId, seatId);
    const auto now = entity_detail::dateTime(clock_());
    requireAvailable(sale, now);
    const Ticket ticket(tickets_.nextId(), screeningId, seatId, now, sale.screening.ticketPrice());
    tickets_.insert(ticket);
    return ticket.id();
}

TicketDetails TicketService::find(EntityId ticketId) const {
    const auto& ticket = service_detail::require(tickets_, ticketId);
    return {ticket, details(ticket.screeningId(), ticket.seatId())};
}

std::unique_ptr<SeatMap> TicketService::seatMap(EntityId screeningId) const {
    const auto& screening = service_detail::require(screenings_, screeningId);
    auto result = makeSeatMap(service_detail::require(rooms_, screening.auditoriumId()), seats_);
    tickets_.forEach([&](const Ticket& ticket) {
        if (ticket.screeningId() != screeningId) {
            return;
        }
        const auto& seat = service_detail::require(seats_, ticket.seatId());
        if (seat.auditoriumId() != screening.auditoriumId()) {
            throw std::invalid_argument("Ticket seat belongs to another room");
        }
        auto& cell = result->at(static_cast<std::size_t>(seat.rowNumber() - 1),
                                static_cast<std::size_t>(seat.seatNumber() - 1));
        if (cell.sold) {
            throw std::invalid_argument("Duplicate screening-seat ticket");
        }
        cell.sold = true;
    });
    return result;
}

void TicketService::validateSaleDate(const std::string& saleDate) {
    screening_time::validateDate(saleDate);
}

DailyRevenue TicketService::revenue(const std::string& saleDate) const {
    validateSaleDate(saleDate);
    DailyRevenue result;
    tickets_.forEach([&](const Ticket& ticket) {
        if (ticket.soldAt().compare(0, 10, saleDate) != 0) {
            return;
        }
        if (ticket.unitPrice() > std::numeric_limits<std::int64_t>::max() - result.amount
            || result.ticketCount == std::numeric_limits<std::size_t>::max()) {
            throw std::overflow_error("Revenue total overflow");
        }
        ++result.ticketCount;
        result.amount += ticket.unitPrice();
    });
    return result;
}
