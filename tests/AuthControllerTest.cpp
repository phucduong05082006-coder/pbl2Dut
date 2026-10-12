#include <cassert>
#include "service/AuthService.h"
#include <chrono>
#include <filesystem>

#include "controller/AuthController.h"

int main()
{
    using namespace cinema;
    const auto root = std::filesystem::temp_directory_path() /
        ("cms_auth_controller_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    ProjectDataEngine engine(root.string());
    std::string error;
    assert(engine.open(error));
    AuthService service(engine);
    AuthController controller(service);
    bool setup = false;
    LoginSession session;
    assert(controller.initialSetupRequired(setup, error) && setup);
    assert(!controller.currentSession(session, error));
    assert(!controller.initializeManager("", "Manager", "secret", error));
    assert(!controller.initializeManager("manager", "Manager", "", error));
    assert(!controller.initializeManager("bad|name", "Manager", "secret", error));
    assert(controller.initialSetupRequired(setup, error) && setup);

    std::filesystem::create_directories(root);
    std::filesystem::create_directory(root / "films.txt.tmp");
    assert(!controller.initializeManager("manager", "Manager", "secret", error));
    assert(controller.initialSetupRequired(setup, error) && setup);
    std::filesystem::remove_all(root);

    assert(controller.initializeManager("manager", "Manager", "secret", error));
    assert(controller.initialSetupRequired(setup, error) && !setup);
    assert(!controller.currentSession(session, error));
    assert(!controller.initializeManager("attacker", "Attacker", "secret", error));
    User manager;
    assert(engine.getUser("U001", manager));
    assert(manager.passwordValue.rfind("$y$", 0) == 0);
    assert(manager.role == UserRole::Manager && !manager.canSellTicket);
    assert(controller.login("manager", "secret", session, error));
    DoublyLinkedList<UserSummary> users;
    assert(controller.listUsers(users, error) && users.size() == 1);
    UserSummary account;
    assert(controller.findUser("U001", account, error));
    assert(account.role == UserRole::Manager);
    assert(!controller.createUser({"U001", "duplicate", "Duplicate", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    assert(!controller.createUser({"U999", "manager", "Duplicate", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    assert(!controller.updateUser({"U999", "missing", "Missing", "", UserRole::Seller, true, RecordStatus::Active}, error));
    assert(!controller.updateUser({"U001", "manager", "Manager", "", UserRole::Seller, true, RecordStatus::Active}, error));
    assert(!controller.updateUser({"U001", "manager", "Manager", "", UserRole::Manager, true, RecordStatus::Inactive}, error));
    assert(controller.updateUser({"U001", "manager", "Manager", "", UserRole::Manager, true, RecordStatus::Active}, error));
    assert(controller.currentSession(session, error) && session.canSellTicket);
    assert(controller.createUser({"", "staff", "Staff", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    assert(controller.listUsers(users, error) && users.size() == 2);
    User staff;
    assert(engine.getUserByUsername("staff", staff));
    const std::string previousHash = staff.passwordValue;
    assert(controller.updateUser({staff.userId, "staff", "New Staff", "", UserRole::Seller, false, RecordStatus::Active}, error));
    assert(engine.getUserByUsername("staff", staff) && staff.passwordValue == previousHash);
    std::filesystem::create_directory(root / "tickets.txt.tmp");
    assert(!controller.updateUser({staff.userId, "staff", "Not saved", "", UserRole::Seller, true, RecordStatus::Active}, error));
    std::filesystem::remove(root / "tickets.txt.tmp");
    assert(controller.findUser(staff.userId, account, error) && account.displayName == "New Staff" && !account.canSellTicket);
    assert(session.userId == "U001");
    assert(!controller.login("manager", "wrong", session, error));
    assert(session.userId.empty());
    assert(!controller.currentSession(session, error));
    assert(controller.login("manager", "secret", session, error));
    assert(service.saveUser({"U002", "seller", "Seller", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    controller.logout();
    assert(!controller.currentSession(session, error));
    assert(controller.login("seller", "secret", session, error));
    assert(session.role == UserRole::Seller && session.canSellTicket);
    assert(!service.requireManager(error));
    assert(!controller.listUsers(users, error) && users.isEmpty());
    assert(!controller.findUser("U001", account, error) && account.userId.empty());
    assert(!controller.createUser({"", "forbidden", "Forbidden", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    assert(!controller.updateUser({"U001", "manager", "Manager", "", UserRole::Seller, true, RecordStatus::Active}, error));
    User seller;
    assert(engine.getUser("U002", seller));
    seller.canSellTicket = false;
    assert(engine.saveUser(seller, error));
    assert(controller.currentSession(session, error) && !session.canSellTicket);
    seller.status = RecordStatus::Inactive;
    assert(engine.saveUser(seller, error));
    assert(!controller.currentSession(session, error));
    assert(session.userId.empty());
    ProjectDataEngine reopened(root.string());
    assert(reopened.open(error));
    AuthService reopenedService(reopened);
    AuthController reopenedController(reopenedService);
    assert(reopenedController.initialSetupRequired(setup, error) && !setup);
    assert(reopenedController.login("manager", "secret", session, error));
    manager.status = RecordStatus::Inactive;
    assert(reopened.saveUser(manager, error));
    assert(!reopenedController.currentSession(session, error));
    assert(reopenedController.initialSetupRequired(setup, error) && !setup);
    assert(!reopenedController.initializeManager("replacement", "Replacement", "secret", error));
    std::filesystem::remove_all(root);
}
