#ifndef CINEMA_DOMAIN_ENUMS_H
#define CINEMA_DOMAIN_ENUMS_H

namespace cinema
{
enum class UserRole
{
    Manager,
    Seller
};

enum class RecordStatus
{
    Active,
    Inactive
};

enum class SeatType
{
    Standard
};

enum class ScreeningStatus
{
    Draft,
    Open,
    Closed,
    Completed
};

enum class PaymentStatus
{
    Unconfirmed,
    Confirmed
};

enum class TransactionStatus
{
    Pending,
    Completed,
    Failed
};

enum class TicketStatus
{
    Issued
};
}

#endif
