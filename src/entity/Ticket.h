#ifndef CINEMA_ENTITY_TICKET_H
#define CINEMA_ENTITY_TICKET_H

#include <string>

#include "../enums/DomainEnums.h"

namespace cinema
{
struct Ticket
{
    std::string ticketId;
    std::string transactionId;
    std::string screeningId;
    std::string seatId;
    long long unitPrice = 0;
    TicketStatus status = TicketStatus::Issued;

    bool operator==(const Ticket& other) const { return ticketId == other.ticketId; }
};
}

#endif
