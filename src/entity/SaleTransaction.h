#ifndef CINEMA_ENTITY_SALE_TRANSACTION_H
#define CINEMA_ENTITY_SALE_TRANSACTION_H

#include <string>

#include "../enums/DomainEnums.h"

namespace cinema
{
struct SaleTransaction
{
    std::string transactionId;
    std::string screeningId;
    std::string createdAt;
    std::string createdBy;
    long long totalAmount = 0;
    PaymentStatus paymentStatus = PaymentStatus::Unconfirmed;
    TransactionStatus status = TransactionStatus::Pending;

    bool operator==(const SaleTransaction& other) const { return transactionId == other.transactionId; }
};
}

#endif
