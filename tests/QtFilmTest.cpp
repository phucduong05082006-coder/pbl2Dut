#include <cassert>
#include "service/AuthService.h"
#include "service/FilmService.h"
#include "service/AuditoriumService.h"
#include "service/ScreeningService.h"
#include "service/SaleService.h"
#include "service/ReportService.h"

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTimer>

#include "ui/mainwindow/MainWindow.h"
#include "ui/Theme.h"

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
    assert(auth.initializeManager("manager", "Manager", "secret", error));
    LoginSession session;
    assert(auth.login("manager", "secret", session, error));
    AuditoriumService auditoriumService(engine, service);
    AuditoriumController auditoriums(auditoriumService);
    ScreeningService screeningService(engine, service);
    SaleService saleService(engine, service);
    ReportService reportService(engine, service);
    OperationsController operations(screeningService, saleService, reportService);
    MainWindow window(auth, films, auditoriums, operations);
    window.show();
    window.findChild<QPushButton*>(QStringLiteral("navFilms"))->click();
    application.processEvents();
    auto* table = window.findChild<QTableWidget*>(QStringLiteral("filmTable"));
    auto* search = window.findChild<QLineEdit*>(QStringLiteral("filmSearch"));
    auto* filter = window.findChild<QComboBox*>(QStringLiteral("filmFilter"));
    auto* add = window.findChild<QPushButton*>(QStringLiteral("filmAdd"));
    auto* edit = window.findChild<QPushButton*>(QStringLiteral("filmEdit"));
    auto* refresh = window.findChild<QPushButton*>(QStringLiteral("filmRefresh"));
    assert(table != nullptr && search != nullptr && filter != nullptr);
    assert(table->rowCount() == 0 && !edit->isEnabled());

    QTimer::singleShot(0, [&] {
        auto* form = window.findChild<QDialog*>(QStringLiteral("filmForm"));
        assert(form != nullptr);
        form->findChild<QLineEdit*>(QStringLiteral("filmId"))->setText(QStringLiteral("F001"));
        form->findChild<QLineEdit*>(QStringLiteral("filmTitle"))->setText(QStringLiteral("Tau tren song"));
        form->findChild<QSpinBox*>(QStringLiteral("filmDuration"))->setValue(98);
        form->findChild<QLineEdit*>(QStringLiteral("filmRating"))->setText(QStringLiteral("P"));
        form->findChild<QPushButton*>(QStringLiteral("filmSave"))->click();
    });
    add->click();
    assert(table->rowCount() == 1);
    Film film;
    assert(films.find("F001", film, error) && film.durationMinutes == 98);

    QTimer::singleShot(0, [&] {
        auto* form = window.findChild<QDialog*>(QStringLiteral("filmForm"));
        form->findChild<QLineEdit*>(QStringLiteral("filmId"))->setText(QStringLiteral("F001"));
        form->findChild<QLineEdit*>(QStringLiteral("filmTitle"))->setText(QStringLiteral("Duplicate"));
        form->findChild<QLineEdit*>(QStringLiteral("filmRating"))->setText(QStringLiteral("P"));
        form->findChild<QPushButton*>(QStringLiteral("filmSave"))->click();
        assert(form->isVisible());
        assert(!form->findChild<QLabel*>(QStringLiteral("filmFormError"))->text().isEmpty());
        form->reject();
    });
    add->click();
    assert(films.find("F001", film, error) && film.title == "Tau tren song");
    table->selectRow(0);
    QTimer::singleShot(0, [&] {
        auto* form = window.findChild<QDialog*>(QStringLiteral("filmForm"));
        assert(form->findChild<QLineEdit*>(QStringLiteral("filmId"))->isReadOnly());
        form->findChild<QLineEdit*>(QStringLiteral("filmTitle"))->setText(QStringLiteral("Tau tren song moi"));
        form->findChild<QPushButton*>(QStringLiteral("filmSave"))->click();
    });
    edit->click();
    assert(films.find("F001", film, error) && film.title == "Tau tren song moi");
    assert(films.create({"F002", "Bau troi xanh", 110, "T13", RecordStatus::Inactive}, error));
    refresh->click();
    assert(table->rowCount() == 2);
    search->setText(QStringLiteral("TAU"));
    assert(QMetaObject::invokeMethod(search, "returnPressed", Qt::DirectConnection));
    assert(table->rowCount() == 1 && table->item(0, 0)->text() == QStringLiteral("F001"));
    search->clear();
    filter->setCurrentIndex(2);
    assert(table->rowCount() == 1 && table->item(0, 0)->text() == QStringLiteral("F002"));
    filter->setCurrentIndex(0);
    table->selectRow(0);
    QTimer::singleShot(0, [&] {
        auto* form = window.findChild<QDialog*>(QStringLiteral("filmForm"));
        assert(form->findChild<QLineEdit*>(QStringLiteral("filmTitle"))->isReadOnly());
        assert(form->findChild<QPushButton*>(QStringLiteral("filmSave")) == nullptr);
        form->reject();
    });
    window.findChild<QPushButton*>(QStringLiteral("filmDetails"))->click();
    QTimer::singleShot(0, [&] {
        auto* confirmation = window.findChild<QMessageBox*>();
        assert(confirmation != nullptr);
        confirmation->button(QMessageBox::Yes)->click();
    });
    window.findChild<QPushButton*>(QStringLiteral("filmDeactivate"))->click();
    assert(films.find("F001", film, error) && film.status == RecordStatus::Inactive);

    assert(service.saveUser({"U002", "seller", "Seller", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    auth.logout();
    assert(auth.login("seller", "secret", session, error));
    assert(QMetaObject::invokeMethod(window.findChild<QTimer*>(), "timeout", Qt::DirectConnection));
    assert(!add->isVisible() && !edit->isVisible());
    assert(!films.create({"F003", "Forbidden", 90, "P", RecordStatus::Active}, error));
    auth.logout();
    refresh->click();
    assert(table->rowCount() == 0);
    assert(!window.findChild<QLabel*>(QStringLiteral("filmError"))->text().isEmpty());

    ProjectDataEngine reloaded(root.path().toStdString());
    assert(reloaded.open(error));
    assert(reloaded.getFilm("F001", film));
    assert(film.title == "Tau tren song moi" && film.status == RecordStatus::Inactive);
}
