#ifndef CINEMA_SERVICE_SALE_SERVICE_H
#define CINEMA_SERVICE_SALE_SERVICE_H

#include "ScreeningService.h"
#include "DataStructures/Matrix2D.h"
#include "../entity/SaleTransaction.h"
#include "../entity/Ticket.h"
#include "../entity/Seat.h"

namespace cinema
{
struct SaleRequest
{
    std::string transactionId;
    std::string screeningId;
    DoublyLinkedList<std::string> seatIds;
    bool paymentConfirmed = false;
};

struct SaleReceipt
{
    SaleTransaction transaction;
    DoublyLinkedList<Ticket> tickets;
};

struct SaleDetails
{
    SaleReceipt receipt;
    ScreeningDetails show;
    DoublyLinkedList<Seat> seats;
    std::string sellerName;
};

class SaleService final
{
public:
    SaleService(ProjectDataEngine& dataEngine, AuthService& auth, ServiceClock clock = {});

    bool preview(const SaleRequest& request, SaleReceipt& receipt, std::string& error) const;

    bool sell(const SaleRequest& request,
              SaleReceipt& receipt,
              std::string& error);
    Matrix2D seatMap(const std::string& screeningId) const;
    bool findTransaction(const std::string& transactionId, SaleTransaction& transaction) const;
    bool findTicket(const std::string& ticketId, Ticket& ticket) const;
    bool describe(const SaleReceipt& receipt, SaleDetails& details, std::string& error) const;
    bool lookup(const std::string& id, bool byTicket, SaleDetails& details, std::string& error) const;

private:
    ProjectDataEngine& dataEngine;
    AuthService& auth;
    ServiceClock clock;
};
}

#endif
