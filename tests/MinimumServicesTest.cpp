#include <cassert>
#include <filesystem>
#include <fstream>

#include "service/AuthService.h"
#include "service/FilmService.h"
#include "service/AuditoriumService.h"
#include "service/ScreeningService.h"
#include "service/SaleService.h"
#include "service/ReportService.h"

namespace
{
void writeSeedData(const std::filesystem::path& root)
{
    std::filesystem::create_directories(root);
    std::ofstream(root / "users.txt")
        << "user_id|username|password_value|display_name|role|can_sell_ticket|status\n"
        << "U001|manager|demo|Quan ly|MANAGER|1|ACTIVE\n"
        << "U002|seller|demo|Nhan vien|SELLER|1|ACTIVE\n"
        << "U003|disabled|demo|Ngung hoat dong|SELLER|1|INACTIVE\n";
    std::ofstream(root / "films.txt")
        << "film_id|title|duration_minutes|age_rating|status\n";
    std::ofstream(root / "auditoriums.txt")
        << "auditorium_id|name|rows|columns|capacity|status\n";
    std::ofstream(root / "seats.txt")
        << "seat_id|auditorium_id|seat_code|row_label|seat_number|seat_type|status\n";
    std::ofstream(root / "screenings.txt")
        << "screening_id|film_id|auditorium_id|start_time|end_time|room_available_until|ticket_price|status\n";
    std::ofstream(root / "transactions.txt")
        << "transaction_id|screening_id|created_at|created_by|total_amount|payment_status|status\n";
    std::ofstream(root / "tickets.txt")
        << "ticket_id|transaction_id|screening_id|seat_id|unit_price|status\n";
}
}

int main()
{
    using namespace cinema;

    const std::filesystem::path root = std::filesystem::temp_directory_path() / "cms_minimum_services_check";
    std::filesystem::remove_all(root);
    writeSeedData(root);

    ProjectDataEngine engine(root.string());
    std::string error;
    assert(engine.open(error));

    AuthService auth(engine);
    std::string now = "2099-10-11 10:00:00";
    ServiceClock clock = [&now] { return now; };
    FilmService films(engine, auth);
    AuditoriumService auditoriums(engine, auth);
    ScreeningService screenings(engine, auth, clock);
    SaleService sales(engine, auth, clock);
    ReportService reports(engine, auth);

    LoginSession session;
    assert(!films.save({"F001", "Denied", 98, "P", RecordStatus::Active}, error));
    assert(auth.login("manager", "demo", session, error));
    assert(session.role == UserRole::Manager);
    User protectedUser;
    assert(engine.getUser("U001", protectedUser));
    assert(protectedUser.passwordValue.rfind("$y$", 0) == 0);
    assert(!auth.login("manager", "wrong", session, error));
    assert(session.userId.empty());
    assert(!auth.requireLogin(error));
    assert(!auth.login("disabled", "demo", session, error));
    assert(auth.login("manager", "demo", session, error));

    assert(auth.saveUser({"U004", "viewer", "Khong ban ve", "secret", UserRole::Seller, false, RecordStatus::Active}, error));
    assert(!auth.saveUser({"U001", "manager", "Quan ly", {}, UserRole::Seller, true, RecordStatus::Active}, error));
    assert(!auth.saveUser({"U005", "viewer", "Trung ten", "secret", UserRole::Seller, true, RecordStatus::Active}, error));
    assert(engine.query<User>().size() == 4);

    assert(films.save({"F001", "Chuyen tau mua thu", 98, "P", RecordStatus::Active}, error));
    assert(films.activeFilms().size() == 1);
    Film film;
    assert(films.find("F001", film));
    assert(film.durationMinutes == 98);

    Auditorium room{"R001", "Phong 01", 1, 2, 2, RecordStatus::Active};
    DoublyLinkedList<Seat> seats;
    seats.addLast({"S001", "R001", "A01", "A", 1, SeatType::Standard, RecordStatus::Active});
    seats.addLast({"S002", "R001", "A02", "A", 2, SeatType::Standard, RecordStatus::Active});
    assert(auditoriums.save(room, seats, error));
    assert(auditoriums.activeAuditoriums().size() == 1);
    Auditorium invalidRoom{"R002", "Invalid", 1, 2, 2, RecordStatus::Active};
    auto duplicatePositions = seats;
    for (Node<Seat>* node = duplicatePositions.getHead(); node != nullptr; node = node->next)
        node->data.auditoriumId = "R002";
    duplicatePositions.getAt(1)->seatNumber = 1;
    assert(!auditoriums.save(invalidRoom, duplicatePositions, error));
    assert(engine.query<Auditorium>().size() == 1);

    CreateScreeningRequest create{"SC001", "F001", "R001", "2099-10-12 19:00:00", 80000, true};
    Screening screening;
    assert(screenings.create(create, screening, error));
    assert(screening.endTime == "2099-10-12 20:38:00");
    assert(screening.roomAvailableUntil == "2099-10-12 20:53:00");
    assert(screenings.openScreenings().size() == 1);
    assert(screenings.openScreeningsFor("F001", "R001", "2099-10-12").size() == 1);
    assert(films.search("tau").size() == 1);
    assert(!screenings.create(create, screening, error));

    create.screeningId = "SC002";
    create.startTime = "2099-10-12 20:00:00";
    assert(!screenings.create(create, screening, error));
    assert(engine.query<Screening>().size() == 1);
    create.startTime = "2099-10-12 20:52:59";
    assert(!screenings.create(create, screening, error));

    create.startTime = "2099-02-31 19:00:00";
    assert(!screenings.create(create, screening, error));
    create.startTime = "2099-10-12 20:53:00";
    create.openForSale = false;
    assert(screenings.create(create, screening, error));
    assert(engine.query<Screening>().size() == 2);
    assert(screenings.openScreenings().size() == 1);
    assert(screenings.changeStatus("SC002", ScreeningStatus::Open, error));
    assert(screenings.openScreenings().size() == 2);
    assert(screenings.changeStatus("SC002", ScreeningStatus::Closed, error));
    assert(!screenings.changeStatus("SC002", ScreeningStatus::Completed, error));

    film.durationMinutes = 99;
    assert(!films.save(film, error));
    film.durationMinutes = 98;
    film.status = RecordStatus::Inactive;
    assert(!films.save(film, error));
    film.status = RecordStatus::Active;
    room.status = RecordStatus::Inactive;
    assert(!auditoriums.save(room, seats, error));
    room.status = RecordStatus::Active;
    auto changedSeats = seats;
    changedSeats.getAt(0)->seatNumber = 2;
    assert(!auditoriums.save(room, changedSeats, error));

    SaleRequest sale;
    sale.transactionId = "TX001";
    sale.screeningId = "SC001";
    sale.seatIds.addLast("S001");
    SaleReceipt receipt;
    assert(sales.preview(sale, receipt, error));
    assert(receipt.transaction.paymentStatus == PaymentStatus::Unconfirmed);
    assert(receipt.transaction.status == TransactionStatus::Pending);
    assert(engine.query<Ticket>().isEmpty());
    assert(!sales.sell(sale, receipt, error));
    assert(receipt.tickets.isEmpty());
    sale.paymentConfirmed = true;
    assert(auth.login("viewer", "secret", session, error));
    assert(!sales.sell(sale, receipt, error));
    assert(auth.login("seller", "demo", session, error));
    assert(!films.save(film, error));
    RevenueReport deniedReport;
    assert(!reports.revenueByFilm({}, deniedReport, error));
    assert(sales.sell(sale, receipt, error));
    assert(receipt.transaction.totalAmount == 80000);
    assert(receipt.tickets.size() == 1);
    assert(receipt.transaction.createdBy == "U002");
    assert(receipt.transaction.createdAt == now);
    assert(sales.seatMap("SC001").count('X') == 1);

    sale.transactionId = "TX002";
    assert(!sales.sell(sale, receipt, error));
    assert(receipt.tickets.isEmpty());
    assert(engine.query<SaleTransaction>().size() == 1);
    assert(engine.query<Ticket>().size() == 1);

    sale.seatIds.clear();
    sale.seatIds.addLast("S002");
    sale.seatIds.addLast("S002");
    assert(!sales.sell(sale, receipt, error));

    sale.seatIds.removeLast();
    sale.transactionId = "BAD|ID";
    assert(!sales.sell(sale, receipt, error));
    assert(engine.query<SaleTransaction>().size() == 1);
    sale.transactionId = "TX002";
    now = "2099-10-12 19:00:00";
    assert(!sales.sell(sale, receipt, error));
    now = "2099-10-11 10:00:00";

    assert(auth.login("manager", "demo", session, error));

    RevenueReport report;
    assert(reports.revenueByFilm({}, report, error));
    assert(report.ticketCount == 1);
    assert(report.revenue == 80000);
    assert(report.rows.size() == 1);
    assert(report.rows.getAt(0)->filmId == "F001");

    assert(reports.revenueByFilm({"2099-10-11 10:00:00", "2099-10-11 10:00:00", "F001", "SC001"}, report, error));
    assert(report.revenue == 80000);
    assert(reports.revenueByFilm({"2099-10-12 00:00:00", {}, {}, {}}, report, error));
    assert(report.revenue == 0 && report.rows.isEmpty());
    assert(!reports.revenueByFilm({"2099-02-31 00:00:00", {}, {}, {}}, report, error));
    assert(!reports.revenueByFilm({"2099-10-13 00:00:00", "2099-10-11 00:00:00", {}, {}}, report, error));
    OccupancyReport occupancy;
    assert(reports.occupancy("SC001", occupancy, error));
    assert(occupancy.soldSeats == 1 && occupancy.availableSeats == 1 && occupancy.percent == 50.0);

    assert(auth.saveUser({"U002", "seller", "Nhan vien", {}, UserRole::Seller, false, RecordStatus::Active}, error));
    assert(auth.login("seller", "demo", session, error));
    assert(!sales.sell(sale, receipt, error));
    auth.logout();
    assert(!auth.currentSession(session, error));
    assert(!sales.findTransaction("TX001", receipt.transaction));
    assert(auth.login("manager", "demo", session, error));
    assert(sales.findTransaction("TX001", receipt.transaction));
    Ticket issued;
    assert(sales.findTicket("TX001-T1", issued));
    Screening protectedScreening;
    assert(engine.getScreening("SC001", protectedScreening));
    protectedScreening.ticketPrice = 90000;
    assert(!engine.saveScreening(protectedScreening, error));
    assert(engine.getScreening("SC001", protectedScreening));
    assert(protectedScreening.ticketPrice == 80000);

    std::filesystem::create_directory(root / "tickets.txt.tmp");
    film.title = "Must not persist";
    assert(!films.save(film, error));
    Film unchanged;
    assert(engine.getFilm("F001", unchanged));
    assert(unchanged.title == "Chuyen tau mua thu");
    ProjectDataEngine afterFailure(root.string());
    assert(afterFailure.open(error));
    assert(afterFailure.getFilm("F001", unchanged));
    assert(unchanged.title == "Chuyen tau mua thu");
    std::filesystem::remove(root / "tickets.txt.tmp");

    assert(auth.saveUser({"U002", "seller", "Nhan vien", {}, UserRole::Seller, true, RecordStatus::Active}, error));
    assert(auth.login("seller", "demo", session, error));
    std::filesystem::create_directory(root / "tickets.txt.tmp");
    assert(!sales.sell(sale, receipt, error));
    assert(receipt.tickets.isEmpty());
    assert(engine.query<Ticket>().size() == 1);
    ProjectDataEngine failedSale(root.string());
    assert(failedSale.open(error));
    assert(failedSale.query<Ticket>().size() == 1);
    assert(failedSale.query<SaleTransaction>().size() == 1);
    std::filesystem::remove(root / "tickets.txt.tmp");

    sale.transactionId.clear();
    assert(sales.sell(sale, receipt, error));
    assert(!receipt.transaction.transactionId.empty());
    assert(receipt.tickets.size() == 1);
    assert(auth.login("manager", "demo", session, error));
    assert(reports.revenueByFilm({}, report, error));
    assert(report.ticketCount == 2 && report.revenue == 160000);
    assert(reports.occupancy("SC001", occupancy, error));
    assert(occupancy.percent == 100.0);

    const char* files[] = {"users.txt", "films.txt", "auditoriums.txt", "seats.txt", "screenings.txt", "transactions.txt", "tickets.txt"};
    for (const char* file : files)
        std::filesystem::copy_file(root / file, root / (std::string(file) + ".bak"), std::filesystem::copy_options::overwrite_existing);
    std::ofstream(root / "write_pending.txt") << "BACKUP_READY\n1\n1\n1\n1\n1\n1\n1\n";
    std::ofstream(root / "films.txt") << "corrupt interrupted write\n";
    ProjectDataEngine recovered(root.string());
    assert(recovered.open(error));
    assert(recovered.getFilm("F001", unchanged));
    assert(unchanged.title == "Chuyen tau mua thu");
    assert(recovered.query<Ticket>().size() == 2);
    assert(!std::filesystem::exists(root / "write_pending.txt"));

    std::ofstream(root / "write_pending.txt") << "COMMITTED\n1\n1\n1\n1\n1\n1\n1\n";
    ProjectDataEngine committed(root.string());
    assert(committed.open(error));
    assert(committed.query<Ticket>().size() == 2);
    assert(!std::filesystem::exists(root / "write_pending.txt"));

    std::ofstream(root / "write_pending.txt") << "BACKUP_READY\n1\n1\n1\n1\n1\n1\n1\n";
    ProjectDataEngine missingBackup(root.string());
    assert(!missingBackup.open(error));
    assert(std::filesystem::exists(root / "write_pending.txt"));
    std::filesystem::remove(root / "write_pending.txt");

    ProjectDataEngine reloaded(root.string());
    assert(reloaded.open(error));
    assert(reloaded.query<Ticket>().size() == 2);
    assert(reloaded.seatMapForScreening("SC001").count('X') == 2);

    std::filesystem::remove_all(root);
    return 0;
}
