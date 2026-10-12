#ifndef CINEMA_ENTITY_USER_H
#define CINEMA_ENTITY_USER_H

#include <string>

#include "../enums/DomainEnums.h"

namespace cinema
{
struct User
{
    std::string userId;
    std::string username;
    std::string passwordValue;
    std::string displayName;
    UserRole role = UserRole::Seller;
    bool canSellTicket = false;
    RecordStatus status = RecordStatus::Active;

    bool operator==(const User& other) const { return userId == other.userId; }
};
}

#endif
