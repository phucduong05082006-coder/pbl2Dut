#include "AuthService.h"
#include "ServiceSupport.h"

#include <crypt.h>

namespace
{
bool validPassword(const std::string& value)
{
    return !value.empty() && value.size() < CRYPT_MAX_PASSPHRASE_SIZE && value.find('\0') == std::string::npos;
}

bool equalSecret(const std::string& left, const std::string& right)
{
    if (left.size() != right.size()) return false;
    unsigned char difference = 0;
    for (std::size_t index = 0; index < left.size(); ++index)
        difference |= static_cast<unsigned char>(left[index]) ^ static_cast<unsigned char>(right[index]);
    return difference == 0;
}

bool passwordMatches(const std::string& stored, const std::string& supplied)
{
    if (!validPassword(supplied) || stored.empty()) return false;
    if (stored[0] != '$') return equalSecret(stored, supplied);
    if (stored.rfind("$y$", 0) != 0) return false;
    struct crypt_data buffer{};
    const char* calculated = crypt_rn(supplied.c_str(), stored.c_str(), &buffer, sizeof(buffer));
    return calculated != nullptr && equalSecret(stored, calculated);
}
}

namespace cinema
{
using namespace detail;

AuthService::AuthService(ProjectDataEngine& engine) : dataEngine(engine) {}

bool AuthService::needsInitialSetup() const
{
    QuerySpec<User> specification;
    specification.limit = 1;
    return dataEngine.query(specification).isEmpty();
}

bool AuthService::initializeManager(const std::string& username, const std::string& displayName,
                                    const std::string& password, std::string& error, bool canSellTicket)
{
    error.clear();
    if (!needsInitialSetup())
    { error = "Da co tai khoan; khong duoc khoi tao lai quan ly."; return false; }
    std::string passwordHash;
    if (!hashPassword(password, passwordHash, error)) return false;
    return dataEngine.saveUser({"U001", username, passwordHash, displayName,
                               UserRole::Manager, canSellTicket, RecordStatus::Active}, error);
}

bool AuthService::hashPassword(const std::string& password, std::string& hash, std::string& error)
{
    error.clear();
    hash.clear();
    if (!validPassword(password)) { error = "Mat khau khong hop le."; return false; }
    char setting[CRYPT_GENSALT_OUTPUT_SIZE]{};
    struct crypt_data buffer{};
    if (crypt_gensalt_rn("$y$", 0, nullptr, 0, setting, sizeof(setting)) == nullptr)
    { error = "Khong tao duoc salt mat khau."; return false; }
    const char* calculated = crypt_rn(password.c_str(), setting, &buffer, sizeof(buffer));
    if (calculated == nullptr) { error = "Khong bam duoc mat khau."; return false; }
    hash = calculated;
    return true;
}

bool AuthService::login(const std::string& username, const std::string& password, LoginSession& session, std::string& error)
{
    error.clear();
    session = {};
    logout();
    User user;
    if (username.empty() || !dataEngine.getUserByUsername(username, user) ||
        user.status != RecordStatus::Active || !passwordMatches(user.passwordValue, password))
    { error = "Thong tin dang nhap khong hop le."; return false; }
    if (user.passwordValue[0] != '$')
    {
        if (!hashPassword(password, user.passwordValue, error) || !dataEngine.saveUser(user, error)) return false;
    }
    currentUserId = user.userId;
    return currentSession(session, error);
}

void AuthService::logout() { currentUserId.clear(); }

bool AuthService::currentSession(LoginSession& session, std::string& error) const
{
    error.clear();
    session = {};
    User user;
    if (currentUserId.empty() || !dataEngine.getUser(currentUserId, user) || user.status != RecordStatus::Active)
    { error = "Nguoi dung chua dang nhap hoac tai khoan bi khoa."; return false; }
    session = {user.userId, user.username, user.displayName, user.role, user.canSellTicket};
    return true;
}

bool AuthService::requireLogin(std::string& error) const
{
    LoginSession session;
    return currentSession(session, error);
}

bool AuthService::requireManager(std::string& error) const
{
    LoginSession session;
    if (!currentSession(session, error)) return false;
    if (session.role == UserRole::Manager) return true;
    error = "Chi quan ly duoc thuc hien thao tac nay.";
    return false;
}

bool AuthService::requireSeller(std::string& error) const
{
    LoginSession session;
    if (!currentSession(session, error)) return false;
    if (session.canSellTicket) return true;
    error = "Tai khoan khong co quyen ban ve.";
    return false;
}

bool AuthService::saveUser(const UserRequest& request, std::string& error)
{
    if (!requireManager(error)) return false;
    if (request.username.find_first_not_of(" \t\r\n\f\v") == std::string::npos ||
        request.displayName.find_first_not_of(" \t\r\n\f\v") == std::string::npos)
    { error = "Ten dang nhap va ten hien thi khong duoc de trong."; return false; }
    User existing;
    const bool updating = dataEngine.getUser(request.userId, existing);
    if (updating && existing.role == UserRole::Manager && existing.status == RecordStatus::Active &&
        (request.role != UserRole::Manager || request.status != RecordStatus::Active))
    {
        QuerySpec<User> specification;
        specification.where = [](const User& user) { return user.role == UserRole::Manager && user.status == RecordStatus::Active; };
        if (dataEngine.query(specification).size() <= 1)
        { error = "Khong duoc khoa hoac ha quyen quan ly cuoi cung."; return false; }
    }
    std::string passwordHash = updating ? existing.passwordValue : std::string();
    if ((!updating || !request.password.empty()) && !hashPassword(request.password, passwordHash, error)) return false;
    return dataEngine.saveUser({request.userId, request.username, passwordHash, request.displayName,
                               request.role, request.canSellTicket, request.status}, error);
}

DoublyLinkedList<UserSummary> AuthService::listUsers() const
{
    std::string error;
    if (!requireManager(error)) throw std::runtime_error(error);
    QuerySpec<User> specification;
    specification.orderBy = [](const User& left, const User& right) { return left.username < right.username; };
    const auto users = dataEngine.query(specification);
    DoublyLinkedList<UserSummary> result;
    for (const Node<User>* node = users.getHead(); node != nullptr; node = node->next)
    {
        const auto& user = node->data;
        result.addLast({user.userId, user.username, user.displayName, user.role, user.canSellTicket, user.status});
    }
    return result;
}

bool AuthService::findUser(const std::string& userId, UserSummary& result, std::string& error) const
{
    result = {};
    if (!requireManager(error)) return false;
    User user;
    if (!dataEngine.getUser(userId, user)) { error = "Khong tim thay tai khoan."; return false; }
    result = {user.userId, user.username, user.displayName, user.role, user.canSellTicket, user.status};
    return true;
}

bool AuthService::createUser(const UserRequest& request, std::string& error)
{
    if (!requireManager(error)) return false;
    UserRequest candidate = request;
    if (candidate.userId.empty()) candidate.userId = newIdentifier("U");
    User existing;
    if (dataEngine.getUser(candidate.userId, existing)) { error = "Ma tai khoan da ton tai."; return false; }
    return saveUser(candidate, error);
}

bool AuthService::updateUser(const UserRequest& request, std::string& error)
{
    if (!requireManager(error)) return false;
    User existing;
    if (!dataEngine.getUser(request.userId, existing)) { error = "Khong tim thay tai khoan."; return false; }
    return saveUser(request, error);
}
}
