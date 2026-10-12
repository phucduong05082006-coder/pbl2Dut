#include "SaleService.h"
#include "ServiceSupport.h"

#include <limits>
#include <utility>

namespace cinema
{
using namespace detail;

SaleService::SaleService(ProjectDataEngine& engine, AuthService& authentication, ServiceClock serviceClock)
    : dataEngine(engine), auth(authentication), clock(std::move(serviceClock)) {}

bool SaleService::preview(const SaleRequest& request, SaleReceipt& receipt, std::string& error) const
{
    receipt = {};
    LoginSession session;
    if (!auth.requireSeller(error) || !auth.currentSession(session, error)) return false;
    Screening screening;
    Film film;
    Auditorium room;
    if (!dataEngine.getScreening(request.screeningId, screening) || screening.status != ScreeningStatus::Open || screening.ticketPrice <= 0 ||
        !dataEngine.getFilm(screening.filmId, film) || film.status != RecordStatus::Active ||
        !dataEngine.getAuditorium(screening.auditoriumId, room) || room.status != RecordStatus::Active)
    { error = "Suat chieu/phim/phong khong san sang de ban ve."; return false; }
    std::string now;
    if (!readTime(clock, now, error)) return false;
    if (request.seatIds.isEmpty() || now >= screening.startTime)
    { error = "Chua chon ghe hoac suat chieu da bat dau."; return false; }
    SaleTransaction existing;
    if (!request.transactionId.empty() && dataEngine.getTransaction(request.transactionId, existing))
    { error = "Ma giao dich da ton tai."; return false; }
    QuerySpec<Ticket> specification;
    specification.where = [&](const Ticket& ticket) { return ticket.screeningId == request.screeningId; };
    const auto soldTickets = dataEngine.query(specification);
    SaleReceipt candidate;
    candidate.transaction = {request.transactionId, request.screeningId, now, session.userId, 0,
                             PaymentStatus::Unconfirmed, TransactionStatus::Pending};
    for (const Node<std::string>* node = request.seatIds.getHead(); node != nullptr; node = node->next)
    {
        Seat seat;
        if (!dataEngine.getSeat(node->data, seat) || seat.status != RecordStatus::Active || seat.auditoriumId != screening.auditoriumId)
        { error = "Ghe khong hop le hoac khong thuoc phong."; return false; }
        for (const Node<Ticket>* previous = candidate.tickets.getHead(); previous != nullptr; previous = previous->next)
            if (previous->data.seatId == node->data) { error = "Khong duoc chon trung ghe."; return false; }
        for (const Node<Ticket>* sold = soldTickets.getHead(); sold != nullptr; sold = sold->next)
            if (sold->data.seatId == node->data) { error = "Ghe da duoc ban."; return false; }
        if (candidate.transaction.totalAmount > std::numeric_limits<long long>::max() - screening.ticketPrice)
        { error = "Tong tien vuot gioi han."; return false; }
        candidate.tickets.addLast({{}, request.transactionId, request.screeningId, node->data, screening.ticketPrice, TicketStatus::Issued});
        candidate.transaction.totalAmount += screening.ticketPrice;
    }
    receipt = candidate;
    return true;
}

bool SaleService::sell(const SaleRequest& request, SaleReceipt& receipt, std::string& error)
{
    receipt = {};
    if (!request.paymentConfirmed) { error = "Chua xac nhan thanh toan."; return false; }
    SaleReceipt candidate;
    if (!preview(request, candidate, error)) return false;
    SaleTransaction existing;
    if (candidate.transaction.transactionId.empty())
        do { candidate.transaction.transactionId = newIdentifier("TX"); }
        while (dataEngine.getTransaction(candidate.transaction.transactionId, existing));
    int number = 1;
    for (Node<Ticket>* node = candidate.tickets.getHead(); node != nullptr; node = node->next, ++number)
    {
        node->data.transactionId = candidate.transaction.transactionId;
        node->data.ticketId = candidate.transaction.transactionId + "-T" + std::to_string(number);
    }
    candidate.transaction.paymentStatus = PaymentStatus::Confirmed;
    candidate.transaction.status = TransactionStatus::Completed;
    if (!dataEngine.commitSale(candidate.transaction, candidate.tickets, error)) return false;
    receipt = candidate;
    return true;
}

Matrix2D SaleService::seatMap(const std::string& screeningId) const
{
    requireRead(auth);
    Screening screening;
    if (!dataEngine.getScreening(screeningId, screening)) throw std::invalid_argument("Khong tim thay suat chieu.");
    return dataEngine.seatMapForScreening(screeningId);
}

bool SaleService::findTransaction(const std::string& id, SaleTransaction& transaction) const
{
    transaction = {};
    std::string error;
    return auth.requireLogin(error) && dataEngine.getTransaction(id, transaction);
}

bool SaleService::findTicket(const std::string& id, Ticket& ticket) const
{
    ticket = {};
    std::string error;
    return auth.requireLogin(error) && dataEngine.getTicket(id, ticket);
}

bool SaleService::describe(const SaleReceipt& receipt, SaleDetails& details, std::string& error) const
{
    details = {};
    if (!auth.requireLogin(error)) return false;
    SaleDetails candidate;
    candidate.receipt = receipt;
    User seller;
    if (!dataEngine.getScreening(receipt.transaction.screeningId, candidate.show.screening) ||
        !dataEngine.getFilm(candidate.show.screening.filmId, candidate.show.film) ||
        !dataEngine.getAuditorium(candidate.show.screening.auditoriumId, candidate.show.auditorium) ||
        !dataEngine.getUser(receipt.transaction.createdBy, seller))
    { error = "Giao dich co tham chieu khong hop le."; return false; }
    candidate.sellerName = seller.displayName;
    for (const Node<Ticket>* node = receipt.tickets.getHead(); node != nullptr; node = node->next)
    {
        Seat seat;
        if (!dataEngine.getSeat(node->data.seatId, seat)) { error = "Khong tim thay ghe cua ve."; return false; }
        candidate.seats.addLast(seat);
    }
    details = candidate;
    return true;
}

bool SaleService::lookup(const std::string& id, bool byTicket, SaleDetails& details, std::string& error) const
{
    details = {};
    if (!auth.requireLogin(error)) return false;
    if (id.find_first_not_of(" \t\r\n") == std::string::npos || id.find_first_of("|\r\n\t") != std::string::npos)
    { error = "Ma tra cuu khong hop le."; return false; }
    SaleReceipt receipt;
    Ticket ticket;
    const std::string transactionId = byTicket && dataEngine.getTicket(id, ticket) ? ticket.transactionId : id;
    if ((byTicket && ticket.ticketId.empty()) || !dataEngine.getTransaction(transactionId, receipt.transaction))
    { error = "Khong tim thay ve/giao dich."; return false; }
    QuerySpec<Ticket> query;
    query.where = [&](const Ticket& value) {
        return value.transactionId == transactionId && (!byTicket || value.ticketId == id);
    };
    receipt.tickets = dataEngine.query(query);
    return describe(receipt, details, error);
}
}
