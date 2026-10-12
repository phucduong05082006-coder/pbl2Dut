#include "AuthController.h"

#include <exception>

namespace cinema
{
AuthController::AuthController(AuthService& authentication) : auth(authentication) {}

bool AuthController::initialSetupRequired(bool& required, std::string& error) const
{
    required = false;
    error.clear();
    try { required = auth.needsInitialSetup(); return true; }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool AuthController::initializeManager(const std::string& username, const std::string& displayName,
                                      const std::string& password, std::string& error, bool canSellTicket)
{
    try { return auth.initializeManager(username, displayName, password, error, canSellTicket); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool AuthController::login(const std::string& username, const std::string& password,
                           LoginSession& session, std::string& error)
{
    session = {};
    try { return auth.login(username, password, session, error); }
    catch (const std::exception& exception)
    { auth.logout(); error = exception.what(); return false; }
}

bool AuthController::currentSession(LoginSession& session, std::string& error) const
{
    session = {};
    try { return auth.currentSession(session, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

void AuthController::logout() { auth.logout(); }

bool AuthController::listUsers(DoublyLinkedList<UserSummary>& result, std::string& error) const
{
    result.clear();
    error.clear();
    try { result = auth.listUsers(); return true; }
    catch (const std::exception& exception) { result.clear(); error = exception.what(); return false; }
}

bool AuthController::findUser(const std::string& userId, UserSummary& result, std::string& error) const
{
    result = {};
    error.clear();
    try { return auth.findUser(userId, result, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool AuthController::createUser(const UserRequest& request, std::string& error)
{
    error.clear();
    try { return auth.createUser(request, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool AuthController::updateUser(const UserRequest& request, std::string& error)
{
    error.clear();
    try { return auth.updateUser(request, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}
}
