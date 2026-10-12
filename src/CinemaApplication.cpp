#include <QApplication>
#include "service/AuthService.h"
#include "service/FilmService.h"
#include "service/AuditoriumService.h"
#include "service/ScreeningService.h"
#include "service/SaleService.h"
#include "service/ReportService.h"
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QLockFile>
#include <QMessageBox>
#include <QStandardPaths>

#include <iostream>

#include "controller/AuthController.h"
#include "controller/FilmController.h"
#include "controller/AuditoriumController.h"
#include "ui/Theme.h"
#include "ui/dialog/LoginDialog.h"
#include "ui/dialog/AdminSetup.h"
#include "ui/mainwindow/MainWindow.h"

namespace
{

bool prepareDemo(cinema::ProjectDataEngine& engine, cinema::AuthService& auth, std::string& error)
{
    cinema::LoginSession session;
    if (!auth.login("manager", "demo123", session, error)) return false;
    cinema::ScreeningService shows(engine, auth);
    cinema::SaleService sales(engine, auth);
    const QDate firstDay = QDate::currentDate().addDays(1);
    const auto films = engine.query<cinema::Film>();
    const auto rooms = engine.query<cinema::Auditorium>();
    if (films.isEmpty() || rooms.isEmpty())
    { error = "Demo source has no films or rooms."; return false; }
    int index = 0;
    for (const Node<cinema::Auditorium>* room = rooms.getHead(); room != nullptr; room = room->next)
    {
        const auto* film = films.getAt(index % films.size());
        for (int day = 0; day < 2; ++day)
        {
            cinema::Screening screening;
            const std::string id = "DEMO_" + std::to_string(index + 1) + "_" + std::to_string(day + 1);
            const std::string start = firstDay.addDays(day).toString(QStringLiteral("yyyy-MM-dd")).toStdString() + " 19:00:00";
            if (!shows.create({id, film->filmId, room->data.auditoriumId, start, 80000, true}, screening, error)) return false;
            if (day == 0)
            {
                cinema::SaleRequest request;
                request.screeningId = id;
                request.paymentConfirmed = true;
                cinema::QuerySpec<cinema::Seat> seats;
                seats.where = [&](const cinema::Seat& seat) { return seat.auditoriumId == room->data.auditoriumId; };
                seats.limit = 2;
                const auto selected = engine.query(seats);
                for (const Node<cinema::Seat>* seat = selected.getHead(); seat != nullptr; seat = seat->next)
                    request.seatIds.addLast(seat->data.seatId);
                cinema::SaleReceipt receipt;
                if (!sales.sell(request, receipt, error)) return false;
            }
        }
        ++index;
    }
    auth.logout();
    return true;
}
}

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    applyCinemaTheme(application);
    application.setApplicationName(QStringLiteral("CinemaManagement"));
    application.setOrganizationName(QStringLiteral("FRAME"));
    application.setQuitOnLastWindowClosed(false);
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("FRAME - Quan ly rap offline"));
    parser.addHelpOption();
    QCommandLineOption dataDirectory(QStringLiteral("data-dir"),
        QStringLiteral("Thu muc du lieu TXT"), QStringLiteral("path"));
    parser.addOption(dataDirectory);
    QCommandLineOption setup(QStringLiteral("setup-admin"), QStringLiteral("Cap Admin dau tien, tach khoi dang nhap"));
    QCommandLineOption demo(QStringLiteral("prepare-demo"), QStringLiteral("Tao du lieu demo trong --data-dir moi; khong ghi de"));
    parser.addOption(setup);
    parser.addOption(demo);
    parser.process(application);
    const QString dataPath = parser.isSet(dataDirectory) ? parser.value(dataDirectory)
        : QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (dataPath.isEmpty())
    {
        QMessageBox::critical(nullptr, QStringLiteral("Lỗi khởi động"), QStringLiteral("Chưa có đường dẫn dữ liệu."));
        return 1;
    }
    if (parser.isSet(demo) && (!parser.isSet(dataDirectory) || parser.isSet(setup)))
    { std::cerr << "Demo requires an explicit --data-dir and cannot use --setup-admin.\n"; return 1; }
    QDir directory(dataPath);
    if (!directory.mkpath(QStringLiteral(".")))
    { std::cerr << "Cannot create data directory.\n"; return 1; }
    QLockFile lock(QFileInfo(directory.absolutePath()).canonicalFilePath() + QStringLiteral("/.cinema.lock"));
    lock.setStaleLockTime(0);
    if (!lock.tryLock(0))
    { std::cerr << "Data directory is already in use or cannot be locked.\n"; return 1; }
    if (parser.isSet(demo))
    {
        const auto existing = directory.entryList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
        if (existing.size() != 1 || existing.first() != QStringLiteral(".cinema.lock"))
        { std::cerr << "Demo requires an empty directory; existing data will not be overwritten.\n"; return 1; }
        const QDir source(QStringLiteral(CMS_DEMO_SOURCE));
        for (const auto& file : source.entryList({QStringLiteral("*.txt")}, QDir::Files))
            if (!QFile::copy(source.filePath(file), directory.filePath(file)))
            { std::cerr << "Cannot copy demo table.\n"; return 1; }
    }
    cinema::ProjectDataEngine dataEngine(directory.absolutePath().toStdString());
    std::string error;
    if (!dataEngine.open(error))
    {
        QMessageBox::critical(nullptr, QStringLiteral("Không thể mở dữ liệu"), QString::fromStdString(error));
        return 1;
    }
    cinema::AuthService authService(dataEngine);
    cinema::AuthController auth(authService);
    if (parser.isSet(setup)) return runAdminSetup(auth) ? 0 : 1;
    if (parser.isSet(demo))
    {
        if (!prepareDemo(dataEngine, authService, error)) { std::cerr << error << '\n'; return 1; }
        std::cout << "Demo ready: " << directory.absolutePath().toStdString() << '\n';
        return 0;
    }
    cinema::FilmService filmService(dataEngine, authService);
    cinema::FilmController films(filmService);
    cinema::AuditoriumService auditoriumService(dataEngine, authService);
    cinema::AuditoriumController auditoriums(auditoriumService);
    cinema::ScreeningService screeningService(dataEngine, authService);
    cinema::SaleService saleService(dataEngine, authService);
    cinema::ReportService reportService(dataEngine, authService);
    cinema::OperationsController operations(screeningService, saleService, reportService);
    for (;;)
    {
        application.setQuitOnLastWindowClosed(false);
        LoginDialog login(auth);
        if (login.exec() != QDialog::Accepted) break;
        MainWindow window(auth, films, auditoriums, operations, QDir(dataPath).absolutePath());
        application.setQuitOnLastWindowClosed(true);
        window.show();
        application.exec();
        cinema::LoginSession session;
        const bool closedByUser = auth.currentSession(session, error);
        auth.logout();
        if (closedByUser) break;
    }
    return 0;
}
