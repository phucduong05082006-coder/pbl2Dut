#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "service/AuthService.h"
#include "service/AuditoriumService.h"
#include "service/SaleService.h"
#include "service/ReportService.h"
#include "util/DateTime.h"

int main()
{
    using namespace cinema;
    const auto root = std::filesystem::temp_directory_path() /
        ("cms_performance_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    ProjectDataEngine engine(root.string());
    std::string error;
    assert(engine.open(error));
    AuthService auth(engine);
    assert(auth.initializeManager("manager", "Manager", "secret", error, true));
    LoginSession session;
    assert(auth.login("manager", "secret", session, error));
    AuditoriumService rooms(engine, auth);
    assert(rooms.saveGrid({"R001", "Room", 6, 8, 0, RecordStatus::Active}, true, error));
    {
        std::ofstream films(root / "films.txt");
        films << "film_id|title|duration_minutes|age_rating|status\n";
        for (int index = 0; index < 1000; ++index)
            films << "F" << index << "|Film " << index << "|90|P|ACTIVE\n";
        std::ofstream shows(root / "screenings.txt");
        shows << "screening_id|film_id|auditorium_id|start_time|end_time|room_available_until|ticket_price|status\n";
        std::ofstream transactions(root / "transactions.txt");
        transactions << "transaction_id|screening_id|created_at|created_by|total_amount|payment_status|status\n";
        std::ofstream tickets(root / "tickets.txt");
        tickets << "ticket_id|transaction_id|screening_id|seat_id|unit_price|status\n";
        std::time_t first;
        assert(parseTimestamp("2099-10-12 19:00:00", first));
        for (int index = 0; index < 100; ++index)
        {
            const std::time_t epoch = first + index * 86400;
            const std::string start = formatTimestamp(*std::localtime(&epoch));
            std::string end, available;
            assert(calculateTimes(start, 90, end, available));
            shows << "SC" << index << "|F" << index << "|R001|" << start << '|' << end << '|' << available << "|80000|OPEN\n";
            transactions << "TX" << index << "|SC" << index << "|2099-10-11 10:00:00|U001|400000|CONFIRMED|COMPLETED\n";
            for (int seat = 1; seat <= 5; ++seat)
                tickets << "T" << index << '_' << seat << "|TX" << index << "|SC" << index << "|R001_A0" << seat << "|80000|ISSUED\n";
        }
    }
    assert(engine.open(error));
    assert(auth.login("manager", "secret", session, error));
    const auto queryStart = std::chrono::steady_clock::now();
    assert(engine.search({SearchEntity::Film, "Film 9", SearchMode::Contains, 0}).size() == 111);
    QuerySpec<Screening> query;
    query.orderBy = [](const Screening& left, const Screening& right) { return left.startTime < right.startTime; };
    assert(engine.query(query).size() == 100);
    ReportService reports(engine, auth);
    RevenueReport report;
    assert(reports.revenueByFilm({}, report, error));
    assert(report.ticketCount == 500 && report.revenue == 40000000);
    const double lookupSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - queryStart).count();
    SaleService sales(engine, auth, [] { return "2099-10-11 10:00:00"; });
    SaleRequest request;
    request.screeningId = "SC0";
    request.paymentConfirmed = true;
    request.seatIds.addLast("R001_A06");
    request.seatIds.addLast("R001_A07");
    SaleReceipt receipt;
    const auto saleStart = std::chrono::steady_clock::now();
    assert(sales.sell(request, receipt, error));
    const double saleSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - saleStart).count();
    std::cout << "1000 films, 1 room, 48 seats, 100 shows, 500 existing tickets: search/query/report="
              << lookupSeconds << "s, two-seat sale=" << saleSeconds << "s\n";
    assert(lookupSeconds <= 2.0 && saleSeconds <= 3.0);
    ProjectDataEngine reopened(root.string());
    assert(reopened.open(error));
    assert(reopened.query<Ticket>().size() == 502);
    std::filesystem::remove_all(root);
}
