#ifndef CINEMA_CONTROLLER_AUTH_CONTROLLER_H
#define CINEMA_CONTROLLER_AUTH_CONTROLLER_H

#include "../service/AuthService.h"

namespace cinema
{
class AuthController final
{
public:
    explicit AuthController(AuthService& auth);

    bool initialSetupRequired(bool& required, std::string& error) const;
    bool initializeManager(const std::string& username, const std::string& displayName,
                           const std::string& password, std::string& error, bool canSellTicket = false);
    bool login(const std::string& username, const std::string& password,
               LoginSession& session, std::string& error);
    bool currentSession(LoginSession& session, std::string& error) const;
    bool listUsers(DoublyLinkedList<UserSummary>& result, std::string& error) const;
    bool findUser(const std::string& userId, UserSummary& result, std::string& error) const;
    bool createUser(const UserRequest& request, std::string& error);
    bool updateUser(const UserRequest& request, std::string& error);
    void logout();

private:
    AuthService& auth;
};
}

#endif
