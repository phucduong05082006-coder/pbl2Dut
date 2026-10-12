#ifndef CINEMA_SERVICE_AUTH_SERVICE_H
#define CINEMA_SERVICE_AUTH_SERVICE_H

#include <string>
#include "../entity/User.h"
#include "../repository/ProjectDataEngine.h"

namespace cinema
{
struct LoginSession
{
    std::string userId;
    std::string username;
    std::string displayName;
    UserRole role = UserRole::Seller;
    bool canSellTicket = false;
};

struct UserRequest
{
    std::string userId;
    std::string username;
    std::string displayName;
    std::string password;
    UserRole role = UserRole::Seller;
    bool canSellTicket = false;
    RecordStatus status = RecordStatus::Active;
};

struct UserSummary
{
    std::string userId;
    std::string username;
    std::string displayName;
    UserRole role = UserRole::Seller;
    bool canSellTicket = false;
    RecordStatus status = RecordStatus::Active;
};

class AuthService final
{
public:
    explicit AuthService(ProjectDataEngine& dataEngine);

    bool needsInitialSetup() const;
    bool initializeManager(const std::string& username,
                           const std::string& displayName,
                           const std::string& password,
                           std::string& error,
                           bool canSellTicket = false);
    bool login(const std::string& username,
               const std::string& password,
               LoginSession& session,
               std::string& error);
    void logout();
    bool currentSession(LoginSession& session, std::string& error) const;
    bool requireLogin(std::string& error) const;
    bool requireManager(std::string& error) const;
    bool requireSeller(std::string& error) const;
    bool saveUser(const UserRequest& request, std::string& error);
    DoublyLinkedList<UserSummary> listUsers() const;
    bool findUser(const std::string& userId, UserSummary& result, std::string& error) const;
    bool createUser(const UserRequest& request, std::string& error);
    bool updateUser(const UserRequest& request, std::string& error);
    static bool hashPassword(const std::string& password, std::string& hash, std::string& error);

private:
    ProjectDataEngine& dataEngine;
    std::string currentUserId;
};
}

#endif
