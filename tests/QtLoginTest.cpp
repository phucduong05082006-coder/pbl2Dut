#include <cassert>
#include "service/AuthService.h"
#include "service/FilmService.h"
#include "service/AuditoriumService.h"
#include "service/ScreeningService.h"
#include "service/SaleService.h"
#include "service/ReportService.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QTimer>

#include "controller/AuthController.h"
#include "controller/FilmController.h"
#include "ui/Theme.h"
#include "ui/dialog/LoginDialog.h"
#include "ui/dialog/AdminSetup.h"
#include "ui/mainwindow/MainWindow.h"
#include "ui/widget/FilmPage.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    applyCinemaTheme(application);
    application.setQuitOnLastWindowClosed(false);
    QTemporaryDir directory;
    assert(directory.isValid());
    cinema::ProjectDataEngine engine(directory.path().toStdString());
    std::string error;
    assert(engine.open(error));
    cinema::AuthService service(engine);
    cinema::AuthController controller(service);
    cinema::FilmService filmService(engine, service);
    cinema::FilmController films(filmService);
    cinema::AuditoriumService auditoriumService(engine, service);
    cinema::AuditoriumController auditoriums(auditoriumService);
    cinema::ScreeningService screeningService(engine, service);
    cinema::SaleService saleService(engine, service);
    cinema::ReportService reportService(engine, service);
    cinema::OperationsController operations(screeningService, saleService, reportService);
    cinema::LoginSession session;

    LoginDialog emptyLogin(controller);
    emptyLogin.show();
    application.processEvents();
    assert(emptyLogin.findChildren<QLineEdit*>().size() == 2);
    assert(emptyLogin.findChild<QLineEdit*>(QStringLiteral("displayName")) == nullptr);
    assert(emptyLogin.findChild<QLineEdit*>(QStringLiteral("confirmation")) == nullptr);
    assert(emptyLogin.findChild<QCheckBox*>(QStringLiteral("initialCanSell")) == nullptr);
    auto* username = emptyLogin.findChild<QLineEdit*>(QStringLiteral("username"));
    auto* password = emptyLogin.findChild<QLineEdit*>(QStringLiteral("password"));
    auto* submit = emptyLogin.findChild<QPushButton*>(QStringLiteral("submit"));
    assert(submit->text() == QStringLiteral("Đăng nhập"));
    username->setText(QStringLiteral("manager"));
    password->setText(QStringLiteral("secret"));
    submit->click();
    assert(emptyLogin.isVisible());
    assert(service.needsInitialSetup());
    assert(!controller.currentSession(session, error));
    assert(!emptyLogin.findChild<QLabel*>(QStringLiteral("error"))->text().isEmpty());
    emptyLogin.close();
    QTimer::singleShot(0, [&] {
        auto* setup = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(setup != nullptr && setup->objectName() == QStringLiteral("adminSetup"));
        setup->reject();
    });
    assert(!runAdminSetup(controller));
    assert(service.needsInitialSetup());
    QTimer::singleShot(0, [&] {
        auto* setup = qobject_cast<QDialog*>(QApplication::activeModalWidget());
        assert(setup != nullptr && setup->objectName() == QStringLiteral("adminSetup"));
        assert(!setup->findChild<QCheckBox*>(QStringLiteral("setupCanSell"))->isChecked());
        setup->findChild<QLineEdit*>(QStringLiteral("setupUsername"))->setText(QStringLiteral("manager"));
        setup->findChild<QLineEdit*>(QStringLiteral("setupDisplayName"))->setText(QStringLiteral("Manager"));
        auto* secret = setup->findChild<QLineEdit*>(QStringLiteral("setupPassword"));
        assert(secret->echoMode() == QLineEdit::Password);
        secret->setText(QStringLiteral("secret"));
        auto* buttons = setup->findChild<QDialogButtonBox*>();
        buttons->button(QDialogButtonBox::Save)->click();
        assert(service.needsInitialSetup() && setup->isVisible());
        setup->findChild<QLineEdit*>(QStringLiteral("setupConfirmation"))->setText(QStringLiteral("secret"));
        buttons->button(QDialogButtonBox::Save)->click();
    });
    assert(runAdminSetup(controller));
    assert(engine.query<cinema::User>().size() == 1);
    QTimer::singleShot(0, [&] {
        auto* message = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
        assert(message != nullptr);
        message->accept();
    });
    assert(!runAdminSetup(controller));
    assert(engine.query<cinema::User>().size() == 1);

    LoginDialog login(controller);
    login.show();
    application.processEvents();
    assert(login.findChildren<QLineEdit*>().size() == 2);
    assert(login.findChild<QLineEdit*>(QStringLiteral("displayName")) == nullptr);
    assert(login.findChild<QLineEdit*>(QStringLiteral("confirmation")) == nullptr);
    if (argc > 1) assert(login.grab().save(QString::fromLocal8Bit(argv[1])));
    username = login.findChild<QLineEdit*>(QStringLiteral("username"));
    password = login.findChild<QLineEdit*>(QStringLiteral("password"));
    submit = login.findChild<QPushButton*>(QStringLiteral("submit"));
    username->setText(QStringLiteral("manager"));
    password->setText(QStringLiteral("wrong"));
    submit->click();
    assert(login.isVisible());
    assert(!controller.currentSession(session, error));
    assert(password->text().isEmpty());
    assert(!login.findChild<QLabel*>(QStringLiteral("error"))->text().isEmpty());
    password->setText(QStringLiteral("secret"));
    submit->click();
    assert(login.result() == QDialog::Accepted);
    assert(password->text().isEmpty());
    assert(controller.currentSession(session, error));
    MainWindow managerWindow(controller, films, auditoriums, operations);
    managerWindow.show();
    managerWindow.findChild<QPushButton*>(QStringLiteral("navFilms"))->click();
    assert(managerWindow.findChild<QLabel*>(QStringLiteral("identity"))->text().contains(QStringLiteral("Manager")));
    assert(managerWindow.findChild<QLabel*>(QStringLiteral("permissions"))->text().contains(QStringLiteral("Admin (Quản lý)")));
    assert(managerWindow.findChild<QLabel*>(QStringLiteral("permissions"))->text().contains(QStringLiteral("Chưa cấp")));
    assert(managerWindow.findChild<QStackedWidget*>(QStringLiteral("workspacePages")) != nullptr);
    auto* filmPage = managerWindow.findChild<QWidget*>(QStringLiteral("filmPage"));
    assert(filmPage != nullptr);
    assert(filmPage->findChild<QPushButton*>(QStringLiteral("filmAdd"))->isVisible());
    assert(service.saveUser({"U002", "seller", "Seller", "secret", cinema::UserRole::Seller,
                             true, cinema::RecordStatus::Active}, error));
    managerWindow.findChild<QPushButton*>(QStringLiteral("logout"))->click();
    assert(!managerWindow.isVisible());
    assert(!controller.currentSession(session, error));
    LoginDialog sellerLogin(controller);
    sellerLogin.show();
    sellerLogin.findChild<QLineEdit*>(QStringLiteral("username"))->setText(QStringLiteral("seller"));
    sellerLogin.findChild<QLineEdit*>(QStringLiteral("password"))->setText(QStringLiteral("secret"));
    sellerLogin.findChild<QPushButton*>(QStringLiteral("submit"))->click();
    assert(sellerLogin.result() == QDialog::Accepted);
    MainWindow sellerWindow(controller, films, auditoriums, operations);
    sellerWindow.show();
    application.processEvents();
    assert(sellerWindow.findChild<QLabel*>(QStringLiteral("identity"))->text().contains(QStringLiteral("Seller")));
    assert(sellerWindow.findChild<QLabel*>(QStringLiteral("permissions"))->text().startsWith(QStringLiteral("Nhân viên")));
    assert(!sellerWindow.findChild<QPushButton*>(QStringLiteral("navAccounts"))->isVisible());
    auto* sellerFilmPage = sellerWindow.findChild<QWidget*>(QStringLiteral("filmPage"));
    assert(sellerFilmPage != nullptr);
    assert(!sellerFilmPage->findChild<QPushButton*>(QStringLiteral("filmAdd"))->isVisible());
    cinema::User seller;
    assert(engine.getUser("U002", seller));
    seller.status = cinema::RecordStatus::Inactive;
    assert(engine.saveUser(seller, error));
    assert(QMetaObject::invokeMethod(sellerWindow.findChild<QTimer*>(), "timeout", Qt::DirectConnection));
    application.processEvents();
    assert(!sellerWindow.isVisible());
    assert(!controller.currentSession(session, error));

    LoginDialog cancel(controller);
    cancel.show();
    QMetaObject::invokeMethod(&cancel, "reject", Qt::DirectConnection);
    assert(cancel.result() == QDialog::Rejected);
    assert(!controller.currentSession(session, error));
}
