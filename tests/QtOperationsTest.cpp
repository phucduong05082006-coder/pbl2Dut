#include <cassert>
#include "service/AuthService.h"
#include "service/FilmService.h"
#include "service/AuditoriumService.h"
#include "service/ScreeningService.h"
#include "service/SaleService.h"
#include "service/ReportService.h"
#include <filesystem>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTextEdit>
#include <QTimer>

#include "ui/Theme.h"
#include "ui/mainwindow/MainWindow.h"
#include "ui/widget/OperationsPages.h"

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
    AuthService authentication(engine);
    AuthController auth(authentication);
    FilmService filmService(engine, authentication);
    FilmController films(filmService);
    AuditoriumService roomService(engine, authentication);
    AuditoriumController rooms(roomService);
    std::string now = "2099-10-11 10:00:00";
    const ServiceClock clock = [&] { return now; };
    ScreeningService screeningService(engine, authentication, clock);
    SaleService saleService(engine, authentication, clock);
    ReportService reportService(engine, authentication);
    OperationsController operations(screeningService, saleService, reportService);
    assert(auth.initializeManager("manager", "Manager", "secret", error, true));
    LoginSession session;
    assert(auth.login("manager", "secret", session, error));
    assert(films.create({"F001", "Test film", 90, "T13", RecordStatus::Active}, error));
    assert(auth.createUser({"U002", "seller", "Seller", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    MainWindow window(auth, films, rooms, operations, root.path());
    window.resize(800, 560);
    window.show();
    QApplication::processEvents();
    assert(window.size() == QSize(800, 560));
    if (argc > 1) window.grab().save(QString::fromLocal8Bit(argv[1]) + QStringLiteral("-small.png"));
    window.resize(1200, 800);
    assert(window.findChild<QPushButton*>(QStringLiteral("navOverview"))->isChecked());
    window.findChild<QPushButton*>(QStringLiteral("navAuditoriums"))->click();
    QTimer::singleShot(0, [&] {
        auto* dialog = window.findChild<QDialog*>(QStringLiteral("auditoriumForm"));
        assert(dialog != nullptr);
        dialog->findChild<QLineEdit*>(QStringLiteral("auditoriumId"))->setText(QStringLiteral("R001"));
        dialog->findChild<QLineEdit*>(QStringLiteral("auditoriumName"))->setText(QStringLiteral("Room one"));
        dialog->findChild<QSpinBox*>(QStringLiteral("auditoriumRows"))->setValue(2);
        dialog->findChild<QSpinBox*>(QStringLiteral("auditoriumColumns"))->setValue(2);
        dialog->findChild<QPushButton*>(QStringLiteral("auditoriumSave"))->click();
    });
    window.findChild<QPushButton*>(QStringLiteral("auditoriumAdd"))->click();
    assert(engine.query<Auditorium>().size() == 1 && engine.query<Seat>().size() == 4);
    assert(!rooms.saveGrid({"R001", "Duplicate", 1, 1, 0, RecordStatus::Active}, true, error));
    window.findChild<QPushButton*>(QStringLiteral("navScreenings"))->click();
    const auto createShow = [&](const QString& id, const QString& start, bool succeeds) {
        QTimer::singleShot(0, [&] {
            auto* dialog = window.findChild<QDialog*>(QStringLiteral("screeningForm"));
            assert(dialog != nullptr);
            dialog->findChild<QLineEdit*>(QStringLiteral("screeningId"))->setText(id);
            dialog->findChild<QDateTimeEdit*>(QStringLiteral("screeningStart"))->setDateTime(
                QDateTime::fromString(start, QStringLiteral("yyyy-MM-dd HH:mm:ss")));
            dialog->findChild<QComboBox*>(QStringLiteral("screeningNewStatus"))->setCurrentIndex(1);
            dialog->findChild<QPushButton*>(QStringLiteral("screeningSave"))->click();
            if (!succeeds) { assert(dialog->isVisible()); dialog->reject(); }
        });
        window.findChild<QPushButton*>(QStringLiteral("screeningAdd"))->click();
    };
    createShow(QStringLiteral("SC001"), QStringLiteral("2099-10-12 19:00:00"), true);
    createShow(QStringLiteral("SC_BAD"), QStringLiteral("2099-10-12 20:44:59"), false);
    createShow(QStringLiteral("SC002"), QStringLiteral("2099-10-12 20:45:00"), true);
    auto* shows = window.findChild<QTableWidget*>(QStringLiteral("screeningTable"));
    assert(shows->rowCount() == 2);
    assert(shows->item(0, 4)->text() == QStringLiteral("2099-10-12 20:30:00"));
    assert(shows->item(0, 5)->text() == QStringLiteral("2099-10-12 20:45:00"));
    shows->selectRow(0);
    window.findChild<QPushButton*>(QStringLiteral("screeningClose"))->click();
    shows->selectRow(0);
    window.findChild<QPushButton*>(QStringLiteral("screeningOpen"))->click();
    assert(!operations.changeStatus("SC001", ScreeningStatus::Completed, error));

    window.findChild<QPushButton*>(QStringLiteral("navSales"))->click();
    auto* choice = window.findChild<QComboBox*>(QStringLiteral("saleScreening"));
    auto* seats = window.findChild<QTableWidget*>(QStringLiteral("saleSeats"));
    auto* preview = window.findChild<QPushButton*>(QStringLiteral("salePreview"));
    assert(choice->count() == 2 && seats->rowCount() == 2);
    seats->item(0, 0)->setSelected(true);
    QTimer::singleShot(0, [&] {
        auto* dialog = window.findChild<QDialog*>(QStringLiteral("saleConfirmation"));
        assert(dialog != nullptr);
        assert(!dialog->findChild<QPushButton*>(QStringLiteral("saleConfirm"))->isEnabled());
        assert(dialog->findChild<QTextEdit*>(QStringLiteral("salePreviewDetails"))->toPlainText().contains(QStringLiteral("80000")));
        dialog->reject();
    });
    preview->click();
    assert(engine.query<Ticket>().isEmpty() && seats->selectedItems().isEmpty());
    seats->item(0, 0)->setSelected(true);
    seats->item(0, 1)->setSelected(true);
    QTimer::singleShot(0, [&] {
        auto* dialog = window.findChild<QDialog*>(QStringLiteral("saleConfirmation"));
        assert(dialog != nullptr);
        assert(dialog->findChild<QTextEdit*>(QStringLiteral("salePreviewDetails"))->toPlainText().contains(QStringLiteral("160000")));
        dialog->findChild<QCheckBox*>(QStringLiteral("salePaid"))->setChecked(true);
        QTimer::singleShot(0, [&] {
            auto* receipt = window.findChild<QDialog*>(QStringLiteral("saleReceipt"));
            assert(receipt != nullptr);
            const auto text = receipt->findChild<QTextEdit*>(QStringLiteral("saleReceiptDetails"))->toPlainText();
            assert(text.contains(QStringLiteral("A01")) && text.contains(QStringLiteral("A02")));
            if (argc > 1) assert(receipt->grab().save(QString::fromLocal8Bit(argv[1]) + QStringLiteral("-receipt.png")));
            receipt->reject();
        });
        dialog->findChild<QPushButton*>(QStringLiteral("saleConfirm"))->click();
    });
    preview->click();
    assert(engine.query<Ticket>().size() == 2 && engine.query<SaleTransaction>().size() == 1);
    assert(!(seats->item(0, 0)->flags() & Qt::ItemIsSelectable));
    if (argc > 1) assert(window.grab().save(QString::fromLocal8Bit(argv[1]) + QStringLiteral("-sale.png")));
    SaleRequest duplicate;
    duplicate.screeningId = "SC001";
    duplicate.seatIds.addLast("R001_A01");
    duplicate.paymentConfirmed = true;
    SaleReceipt result;
    assert(!operations.sell(duplicate, result, error));

    const auto transaction = engine.query<SaleTransaction>().getHead()->data;
    const auto ticket = engine.query<Ticket>().getHead()->data;
    window.findChild<QPushButton*>(QStringLiteral("navLookup"))->click();
    window.findChild<QLineEdit*>(QStringLiteral("lookupId"))->setText(QString::fromStdString(transaction.transactionId));
    window.findChild<QPushButton*>(QStringLiteral("lookupSearch"))->click();
    const auto lookupText = window.findChild<QTextEdit*>(QStringLiteral("lookupResult"))->toPlainText();
    assert(lookupText.contains(QStringLiteral("Test film")) && lookupText.contains(QStringLiteral("Room one")) &&
           lookupText.contains(QStringLiteral("Manager")) && lookupText.contains(QStringLiteral("160000")));
    window.findChild<QComboBox*>(QStringLiteral("lookupType"))->setCurrentIndex(1);
    window.findChild<QLineEdit*>(QStringLiteral("lookupId"))->setText(QString::fromStdString(ticket.ticketId));
    window.findChild<QPushButton*>(QStringLiteral("lookupSearch"))->click();
    assert(window.findChild<QTextEdit*>(QStringLiteral("lookupResult"))->toPlainText().contains(QString::fromStdString(ticket.ticketId)));
    window.findChild<QLineEdit*>(QStringLiteral("lookupId"))->setText(QStringLiteral("missing"));
    window.findChild<QPushButton*>(QStringLiteral("lookupSearch"))->click();
    assert(window.findChild<QTextEdit*>(QStringLiteral("lookupResult"))->toPlainText().isEmpty());

    window.findChild<QPushButton*>(QStringLiteral("navReports"))->click();
    window.findChild<QDateEdit*>(QStringLiteral("reportFrom"))->setDate(QDate(2099, 10, 11));
    window.findChild<QDateEdit*>(QStringLiteral("reportTo"))->setDate(QDate(2099, 10, 11));
    window.findChild<QPushButton*>(QStringLiteral("reportRefresh"))->click();
    auto* report = window.findChild<QTableWidget*>(QStringLiteral("reportTable"));
    assert(report->rowCount() == 1 && report->item(0, 2)->text() == QStringLiteral("2") && report->item(0, 3)->text() == QStringLiteral("160000"));
    auto* filled = window.findChild<QTableWidget*>(QStringLiteral("reportOccupancy"));
    assert(filled->rowCount() == 2 && filled->item(0, 5)->text() == QStringLiteral("50.00"));
    auto* filmFilter = window.findChild<QComboBox*>(QStringLiteral("reportFilm"));
    auto* showFilter = window.findChild<QComboBox*>(QStringLiteral("reportScreening"));
    assert(filmFilter != nullptr && showFilter != nullptr);
    filmFilter->setCurrentIndex(filmFilter->findData(QStringLiteral("F001")));
    showFilter->setCurrentIndex(showFilter->findData(QStringLiteral("SC001")));
    window.findChild<QPushButton*>(QStringLiteral("reportRefresh"))->click();
    assert(report->rowCount() == 1 && filled->rowCount() == 1);
    showFilter->setCurrentIndex(showFilter->findData(QStringLiteral("SC002")));
    window.findChild<QPushButton*>(QStringLiteral("reportRefresh"))->click();
    assert(report->rowCount() == 0 && filled->rowCount() == 1 && filled->item(0, 5)->text() == QStringLiteral("0.00"));
    showFilter->setCurrentIndex(0);
    window.findChild<QDateEdit*>(QStringLiteral("reportFrom"))->setDate(QDate(2099, 10, 13));
    window.findChild<QPushButton*>(QStringLiteral("reportRefresh"))->click();
    assert(report->rowCount() == 0 && filled->rowCount() == 0);
    window.findChild<QDateEdit*>(QStringLiteral("reportFrom"))->setDate(QDate(2099, 10, 11));
    window.findChild<QPushButton*>(QStringLiteral("reportRefresh"))->click();
    if (argc > 1) assert(window.grab().save(QString::fromLocal8Bit(argv[1]) + QStringLiteral("-report.png")));

    window.findChild<QPushButton*>(QStringLiteral("navSales"))->click();
    choice->setCurrentIndex(choice->findData(QStringLiteral("SC002")));
    seats->item(0, 0)->setSelected(true);
    std::filesystem::create_directory(std::filesystem::path(root.path().toStdString()) / "tickets.txt.tmp");
    QTimer::singleShot(0, [&] {
        auto* dialog = window.findChild<QDialog*>(QStringLiteral("saleConfirmation"));
        dialog->findChild<QCheckBox*>(QStringLiteral("salePaid"))->setChecked(true);
        dialog->findChild<QPushButton*>(QStringLiteral("saleConfirm"))->click();
        assert(dialog->isVisible());
        dialog->reject();
    });
    preview->click();
    assert(engine.query<Ticket>().size() == 2);
    std::filesystem::remove(std::filesystem::path(root.path().toStdString()) / "tickets.txt.tmp");
    duplicate.screeningId = "SC002";
    assert(operations.sell(duplicate, result, error));
    assert(engine.query<Ticket>().size() == 3);
    assert(!rooms.saveGrid({"R001", "Room one", 2, 2, 0, RecordStatus::Inactive}, false, error));
    Film protectedFilm;
    assert(filmService.find("F001", protectedFilm));
    protectedFilm.status = RecordStatus::Inactive;
    assert(!films.update(protectedFilm, error));

    assert(auth.login("seller", "secret", session, error));
    assert(QMetaObject::invokeMethod(window.findChild<QTimer*>(), "timeout", Qt::DirectConnection));
    assert(!window.findChild<QPushButton*>(QStringLiteral("navReports"))->isVisible());
    assert(!window.findChild<QPushButton*>(QStringLiteral("navScreenings"))->isVisible());
    assert(!window.findChild<QPushButton*>(QStringLiteral("auditoriumAdd"))->isVisible());
    RevenueReport denied;
    assert(!operations.revenue({}, denied, error));
    assert(!operations.changeStatus("SC002", ScreeningStatus::Closed, error));
    window.findChild<QPushButton*>(QStringLiteral("navSales"))->click();
    choice->setCurrentIndex(choice->findData(QStringLiteral("SC002")));
    seats->item(0, 1)->setSelected(true);
    QTimer::singleShot(0, [&] {
        auto* dialog = window.findChild<QDialog*>(QStringLiteral("saleConfirmation"));
        User seller;
        assert(engine.getUser("U002", seller));
        seller.canSellTicket = false;
        assert(engine.saveUser(seller, error));
        dialog->findChild<QCheckBox*>(QStringLiteral("salePaid"))->setChecked(true);
        dialog->findChild<QPushButton*>(QStringLiteral("saleConfirm"))->click();
        assert(dialog->isVisible());
        dialog->reject();
    });
    preview->click();
    assert(engine.query<Ticket>().size() == 3);
    now = "2099-10-12 20:45:00";
    window.findChild<QPushButton*>(QStringLiteral("saleRefresh"))->click();
    assert(choice->count() == 0);
    assert(auth.login("manager", "secret", session, error));
    duplicate.seatIds.clear();
    duplicate.seatIds.addLast("R001_B01");
    assert(!operations.sell(duplicate, result, error));
    assert(!operations.changeStatus("SC002", ScreeningStatus::Completed, error));
    now = "2099-10-12 22:15:00";
    assert(operations.changeStatus("SC002", ScreeningStatus::Completed, error));
    assert(!operations.changeStatus("SC002", ScreeningStatus::Open, error));
    DoublyLinkedList<ScreeningDetails> invalidFilter;
    assert(!operations.screenings({}, {}, "2099-02-31", std::nullopt, false, invalidFilter, error));
    assert(invalidFilter.isEmpty() && !error.empty());
    window.close();

    ProjectDataEngine reopened(root.path().toStdString());
    assert(reopened.open(error));
    assert(reopened.query<Ticket>().size() == 3 && reopened.query<SaleTransaction>().size() == 2);
    assert(reopened.seatMapForScreening("SC001").count('X') == 2);
    assert(reopened.seatMapForScreening("SC002").count('X') == 1);
}
