#include <cassert>
#include "service/AuthService.h"
#include "service/FilmService.h"
#include "service/AuditoriumService.h"
#include "service/ScreeningService.h"
#include "service/SaleService.h"
#include "service/ReportService.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTimer>

#include "ui/Theme.h"
#include "ui/mainwindow/MainWindow.h"

void checkFilmLayout(MainWindow& window)
{
    QApplication::processEvents();
    const char* names[] = {"filmSearch", "filmFilter", "filmAdd", "filmEdit", "filmDetails",
                           "filmDeactivate", "filmRefresh", "filmTable"};
    for (const char* name : names)
    {
        auto* control = window.findChild<QWidget*>(QString::fromLatin1(name));
        assert(control != nullptr && control->height() >= 40);
        const QRect geometry(control->mapTo(&window, QPoint()), control->size());
        for (const char* otherName : names)
        {
            if (name == otherName) continue;
            auto* other = window.findChild<QWidget*>(QString::fromLatin1(otherName));
            const QRect otherGeometry(other->mapTo(&window, QPoint()), other->size());
            assert(!geometry.intersects(otherGeometry));
        }
    }
}

int main(int argc, char* argv[])
{
    using namespace cinema;
    QApplication application(argc, argv);
    applyCinemaTheme(application);
    QTemporaryDir root;
    assert(root.isValid());
    ProjectDataEngine engine(root.path().toStdString());
    std::string error;
    assert(engine.open(error));
    AuthService service(engine);
    AuthController auth(service);
    FilmService filmService(engine, service);
    FilmController films(filmService);
    AuditoriumService auditoriumService(engine, service);
    AuditoriumController auditoriums(auditoriumService);
    ScreeningService screeningService(engine, service);
    SaleService saleService(engine, service);
    ReportService reportService(engine, service);
    OperationsController operations(screeningService, saleService, reportService);
    assert(auth.initializeManager("admin", "Admin", "secret", error, true));
    LoginSession session;
    assert(auth.login("admin", "secret", session, error));
    MainWindow window(auth, films, auditoriums, operations);
    window.show();
    window.findChild<QPushButton*>(QStringLiteral("navFilms"))->click();
    checkFilmLayout(window);
    assert(window.findChild<QLabel*>(QStringLiteral("permissions"))->text().contains(QStringLiteral("Bán vé:")));
    window.resize(800, 560);
    checkFilmLayout(window);
    window.resize(1200, 800);
    auto* accounts = window.findChild<QPushButton*>(QStringLiteral("navAccounts"));
    assert(accounts->isVisible());
    accounts->click();
    QApplication::processEvents();
    auto* table = window.findChild<QTableWidget*>(QStringLiteral("accountTable"));
    auto* add = window.findChild<QPushButton*>(QStringLiteral("accountAdd"));
    auto* edit = window.findChild<QPushButton*>(QStringLiteral("accountEdit"));
    assert(table->rowCount() == 1);
    assert(table->item(0, 2)->text() == QStringLiteral("Admin (Quản lý)"));
    QTimer::singleShot(0, [&] {
        auto* form = window.findChild<QDialog*>(QStringLiteral("accountForm"));
        assert(form != nullptr);
        form->findChild<QLineEdit*>(QStringLiteral("accountUsername"))->setText(QStringLiteral("seller01"));
        form->findChild<QLineEdit*>(QStringLiteral("accountDisplayName"))->setText(QStringLiteral("Nhân viên 01"));
        form->findChild<QLineEdit*>(QStringLiteral("accountPassword"))->setText(QStringLiteral("secret"));
        form->findChild<QLineEdit*>(QStringLiteral("accountConfirmation"))->setText(QStringLiteral("wrong"));
        auto* save = form->findChild<QPushButton*>(QStringLiteral("accountSave"));
        save->click();
        assert(form->isVisible() && !form->findChild<QLabel*>(QStringLiteral("accountFormError"))->text().isEmpty());
        form->findChild<QLineEdit*>(QStringLiteral("accountConfirmation"))->setText(QStringLiteral("secret"));
        save->click();
    });
    add->click();
    assert(table->rowCount() == 2);
    assert(table->item(1, 2)->text() == QStringLiteral("Nhân viên"));
    assert(table->item(1, 3)->text() == QStringLiteral("Được cấp"));
    User seller;
    assert(engine.getUserByUsername("seller01", seller));
    const std::string oldHash = seller.passwordValue;
    table->selectRow(1);
    QTimer::singleShot(0, [&] {
        auto* form = window.findChild<QDialog*>(QStringLiteral("accountForm"));
        assert(form->findChild<QLineEdit*>(QStringLiteral("accountPassword"))->text().isEmpty());
        form->findChild<QLineEdit*>(QStringLiteral("accountDisplayName"))->setText(QStringLiteral("Nhân viên đã sửa"));
        form->findChild<QPushButton*>(QStringLiteral("accountSave"))->click();
    });
    edit->click();
    assert(engine.getUserByUsername("seller01", seller));
    assert(seller.passwordValue == oldHash && seller.displayName == "Nhân viên đã sửa");
    table->selectRow(1);
    QTimer::singleShot(0, [&] {
        auto* confirmation = window.findChild<QMessageBox*>();
        assert(confirmation != nullptr);
        confirmation->button(QMessageBox::Yes)->click();
    });
    window.findChild<QPushButton*>(QStringLiteral("accountPermission"))->click();
    assert(engine.getUserByUsername("seller01", seller) && !seller.canSellTicket);
    table->selectRow(0);
    QTimer::singleShot(0, [&] {
        auto* confirmation = window.findChild<QMessageBox*>();
        confirmation->button(QMessageBox::Yes)->click();
    });
    window.findChild<QPushButton*>(QStringLiteral("accountStatus"))->click();
    assert(!window.findChild<QLabel*>(QStringLiteral("accountError"))->text().isEmpty());
    User admin;
    assert(engine.getUserByUsername("admin", admin) && admin.status == RecordStatus::Active);
    table->selectRow(1);
    QTimer::singleShot(0, [&] {
        auto* form = window.findChild<QDialog*>(QStringLiteral("accountForm"));
        form->findChild<QComboBox*>(QStringLiteral("accountRole"))->setCurrentIndex(1);
        form->findChild<QCheckBox*>(QStringLiteral("accountCanSell"))->setChecked(true);
        form->findChild<QPushButton*>(QStringLiteral("accountSave"))->click();
    });
    edit->click();
    assert(table->item(1, 2)->text() == QStringLiteral("Admin (Quản lý)"));
    table->selectRow(1);
    QTimer::singleShot(0, [&] {
        auto* form = window.findChild<QDialog*>(QStringLiteral("accountForm"));
        form->findChild<QComboBox*>(QStringLiteral("accountRole"))->setCurrentIndex(0);
        form->findChild<QPushButton*>(QStringLiteral("accountSave"))->click();
    });
    edit->click();
    assert(auth.login("seller01", "secret", session, error));
    assert(session.role == UserRole::Seller && session.canSellTicket);
    assert(QMetaObject::invokeMethod(window.findChild<QTimer*>(), "timeout", Qt::DirectConnection));
    assert(!accounts->isVisible());
    assert(table->rowCount() == 0);
    assert(window.findChild<QStackedWidget*>(QStringLiteral("workspacePages"))->currentIndex() == 4);
    assert(!window.findChild<QPushButton*>(QStringLiteral("filmAdd"))->isVisible());
    DoublyLinkedList<UserSummary> users;
    assert(!auth.listUsers(users, error) && users.isEmpty());
    assert(!auth.updateUser({seller.userId, "seller01", "Escalation", "", UserRole::Manager, true, RecordStatus::Active}, error));
    assert(auth.login("admin", "secret", session, error));
    assert(auth.updateUser({seller.userId, "seller01", "Staff", "", UserRole::Seller, true, RecordStatus::Inactive}, error));
    assert(!auth.login("seller01", "secret", session, error));
    ProjectDataEngine reopened(root.path().toStdString());
    assert(reopened.open(error));
    assert(reopened.getUserByUsername("seller01", seller) && seller.status == RecordStatus::Inactive);
}
