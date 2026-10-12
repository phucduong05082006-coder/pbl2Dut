#include <cassert>
#include "service/AuthService.h"
#include "service/FilmService.h"
#include "service/AuditoriumService.h"
#include "service/ScreeningService.h"
#include "service/SaleService.h"
#include "service/ReportService.h"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTemporaryDir>

#include "ui/Theme.h"
#include "ui/mainwindow/MainWindow.h"
#include "ui/widget/AuditoriumPage.h"

int main(int argc, char* argv[])
{
    using namespace cinema;
    QApplication application(argc, argv);
    applyCinemaTheme(application);
    QTemporaryDir root;
    assert(root.isValid());
    const char* files[] = {"films.txt", "auditoriums.txt", "seats.txt", "users.txt", "screenings.txt",
                           "transactions.txt", "tickets.txt"};
    for (const char* name : files)
        assert(QFile::copy(QDir(QString::fromUtf8(CMS_DEMO_PATH)).filePath(QString::fromLatin1(name)),
                           QDir(root.path()).filePath(QString::fromLatin1(name))));
    ProjectDataEngine engine(root.path().toStdString());
    std::string error;
    assert(engine.open(error));
    assert(engine.query<Film>().size() == 3);
    assert(engine.query<Auditorium>().size() == 3);
    assert(engine.query<Seat>().size() == 136);
    assert(engine.query<User>().size() == 2);
    AuthService authentication(engine);
    AuthController auth(authentication);
    FilmService filmService(engine, authentication);
    FilmController films(filmService);
    AuditoriumService service(engine, authentication);
    AuditoriumController auditoriums(service);
    ScreeningService screeningService(engine, authentication);
    SaleService saleService(engine, authentication);
    ReportService reportService(engine, authentication);
    OperationsController operations(screeningService, saleService, reportService);
    DoublyLinkedList<Auditorium> rooms;
    assert(!auditoriums.list(std::nullopt, rooms, error) && rooms.isEmpty());
    LoginSession session;
    assert(auth.login("manager", "demo123", session, error));
    assert(session.role == UserRole::Manager && session.canSellTicket);
    assert(auditoriums.list(std::nullopt, rooms, error) && rooms.size() == 3);
    assert(!auditoriums.list(static_cast<RecordStatus>(99), rooms, error) && rooms.isEmpty());
    Auditorium room;
    DoublyLinkedList<Seat> seats;
    assert(!auditoriums.details("missing", room, seats, error));
    assert(auditoriums.details("R001", room, seats, error) && seats.size() == 48);

    MainWindow window(auth, films, auditoriums, operations, root.path());
    window.show();
    assert(window.findChild<QLabel*>(QStringLiteral("dataLocation"))->text().contains(root.path()));
    assert(window.findChild<QTableWidget*>(QStringLiteral("filmTable"))->rowCount() == 3);
    auto* navigation = window.findChild<QPushButton*>(QStringLiteral("navAuditoriums"));
    navigation->click();
    QApplication::processEvents();
    auto* choice = window.findChild<QComboBox*>(QStringLiteral("auditoriumChoice"));
    auto* filter = window.findChild<QComboBox*>(QStringLiteral("auditoriumFilter"));
    auto* table = window.findChild<QTableWidget*>(QStringLiteral("auditoriumSeats"));
    assert(choice->count() == 3 && table->rowCount() == 6 && table->columnCount() == 8);
    assert(table->item(0, 0)->text() == QStringLiteral("A01"));
    assert(table->item(5, 7)->text() == QStringLiteral("F08"));
    assert(table->item(0, 0)->data(Qt::UserRole).toString() == QStringLiteral("R001_A01"));
    if (argc > 1) assert(window.grab().save(QString::fromLocal8Bit(argv[1])));
    window.resize(800, 560);
    QApplication::processEvents();
    assert(choice->height() >= 40 && filter->height() >= 40);
    assert(!choice->geometry().intersects(filter->geometry()));
    if (argc > 2) assert(window.grab().save(QString::fromLocal8Bit(argv[2])));
    choice->setCurrentIndex(choice->findData(QStringLiteral("R002")));
    assert(table->rowCount() == 5 && table->columnCount() == 8);
    assert(table->item(4, 7)->text() == QStringLiteral("E08"));
    filter->setCurrentIndex(2);
    assert(choice->count() == 0 && table->rowCount() == 0);
    filter->setCurrentIndex(0);
    assert(choice->count() == 3 && table->rowCount() == 6);

    assert(auditoriums.details("R001", room, seats, error));
    seats.getHead()->data.status = RecordStatus::Inactive;
    assert(service.save(room, seats, error));
    window.findChild<QPushButton*>(QStringLiteral("auditoriumRefresh"))->click();
    assert(table->item(0, 0)->text().contains(QStringLiteral("[Ngừng]")));
    assert(table->item(0, 1)->text() == QStringLiteral("A02"));
    assert(auth.createUser({"", "staff", "Staff", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    auth.logout();
    assert(auth.login("staff", "secret", session, error));
    navigation->click();
    assert(choice->count() == 3 && table->rowCount() == 6);
    assert(!service.save(room, seats, error));
    auth.logout();
    static_cast<AuditoriumPage*>(table->parentWidget())->refreshPermissions();
    assert(choice->count() == 0 && table->rowCount() == 0 && !choice->isEnabled());
    assert(!auditoriums.details("R001", room, seats, error) && seats.isEmpty() && room.auditoriumId.empty());
    ProjectDataEngine reopened(root.path().toStdString());
    assert(reopened.open(error) && reopened.query<Seat>().size() == 136);
    AuthService demoAuth(reopened);
    assert(demoAuth.login("seller01", "demo123", session, error));
    assert(session.role == UserRole::Seller && session.canSellTicket);
    QFile roomFile(QDir(root.path()).filePath(QStringLiteral("auditoriums.txt")));
    assert(roomFile.open(QIODevice::ReadOnly));
    QByteArray contents = roomFile.readAll();
    roomFile.close();
    contents.replace("|6|8|48|", "|999|8|48|");
    assert(roomFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    assert(roomFile.write(contents) == contents.size());
    roomFile.close();
    assert(!reopened.open(error));
    assert(!error.empty() && reopened.query<Auditorium>().isEmpty());
}
