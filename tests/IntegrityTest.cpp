#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>

#include "service/AuthService.h"
#include "service/ReportService.h"

namespace fs = std::filesystem;

std::string readFile(const fs::path& file)
{
    std::ifstream input(file);
    return std::string(std::istreambuf_iterator<char>(input), {});
}

void replaceText(const fs::path& file, const std::string& from, const std::string& to)
{
    std::string text = readFile(file);
    const auto offset = text.find(from);
    assert(offset != std::string::npos);
    text.replace(offset, from.size(), to);
    std::ofstream(file) << text;
}

int main()
{
    using namespace cinema;
    const auto root = fs::temp_directory_path() /
        ("cms_integrity_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto seed = root / "seed";
    fs::create_directories(seed);
    std::ofstream(seed / "users.txt")
        << "user_id|username|password_value|display_name|role|can_sell_ticket|status\n"
        << "U001|manager|secret|Manager|MANAGER|1|ACTIVE\n";
    std::ofstream(seed / "films.txt")
        << "film_id|title|duration_minutes|age_rating|status\nF001|Film|90|P|ACTIVE\n";
    std::ofstream(seed / "auditoriums.txt")
        << "auditorium_id|name|rows|columns|capacity|status\nR001|Room|2|2|4|ACTIVE\n";
    std::ofstream(seed / "seats.txt")
        << "seat_id|auditorium_id|seat_code|row_label|seat_number|seat_type|status\n"
        << "S001|R001|A01|A|1|STANDARD|ACTIVE\nS002|R001|A02|A|2|STANDARD|ACTIVE\n"
        << "S003|R001|B01|B|1|STANDARD|ACTIVE\nS004|R001|B02|B|2|STANDARD|INACTIVE\n";
    std::ofstream(seed / "screenings.txt")
        << "screening_id|film_id|auditorium_id|start_time|end_time|room_available_until|ticket_price|status\n"
        << "SC001|F001|R001|2099-10-12 19:00:00|2099-10-12 20:30:00|2099-10-12 20:45:00|80000|OPEN\n";
    std::ofstream(seed / "transactions.txt")
        << "transaction_id|screening_id|created_at|created_by|total_amount|payment_status|status\n"
        << "TX001|SC001|2099-10-11 10:00:00|U001|80000|CONFIRMED|COMPLETED\n";
    std::ofstream(seed / "tickets.txt")
        << "ticket_id|transaction_id|screening_id|seat_id|unit_price|status\n"
        << "T001|TX001|SC001|S001|80000|ISSUED\n";
    ProjectDataEngine valid(seed.string());
    std::string error;
    assert(valid.open(error));
    assert(valid.seatMapForScreening("SC001").count('X') == 1);
    assert(valid.seatMapForScreening("SC001").count('O') == 2);
    AuthService auth(valid);
    LoginSession session;
    assert(auth.login("manager", "secret", session, error));
    ReportService reports(valid, auth);
    OccupancyReport occupancy;
    assert(reports.occupancy("SC001", occupancy, error));
    assert(occupancy.soldSeats == 1 && occupancy.availableSeats == 3 && occupancy.percent == 25.0);

    const auto longerBreak = root / "longer_break";
    fs::copy(seed, longerBreak, fs::copy_options::recursive);
    replaceText(longerBreak / "screenings.txt", "20:45:00", "20:46:00");
    ProjectDataEngine extended(longerBreak.string());
    assert(extended.open(error));

    int number = 0;
    const auto rejected = [&](const std::function<void(const fs::path&)>& corrupt) {
        const auto target = root / ("invalid" + std::to_string(++number));
        fs::copy(seed, target, fs::copy_options::recursive);
        corrupt(target);
        const std::string before = readFile(target / "users.txt");
        ProjectDataEngine engine(target.string());
        assert(!engine.open(error));
        assert(!error.empty() && engine.query<Ticket>().isEmpty());
        assert(readFile(target / "users.txt") == before);
    };
    rejected([](const auto& target) { fs::remove(target / "tickets.txt"); });
    rejected([](const auto& target) { std::ofstream(target / "tickets.txt"); });
    rejected([](const auto& target) {
        std::ofstream(target / "tickets.txt") << "ticket_id|transaction_id|screening_id|seat_id|unit_price|status\n";
    });
    rejected([](const auto& target) { replaceText(target / "transactions.txt", "|80000|", "|1|"); });
    rejected([](const auto& target) { replaceText(target / "transactions.txt", "CONFIRMED|COMPLETED", "UNCONFIRMED|FAILED"); });
    rejected([](const auto& target) { replaceText(target / "transactions.txt", "CONFIRMED|COMPLETED", "UNCONFIRMED|COMPLETED"); });
    rejected([](const auto& target) { replaceText(target / "transactions.txt", "2099-10-11 10:00:00", "2099-10-12 19:00:00"); });
    rejected([](const auto& target) { replaceText(target / "tickets.txt", "|80000|", "|-80000|"); });
    rejected([](const auto& target) { replaceText(target / "tickets.txt", "SC001|S001", "SC001|missing"); });
    rejected([](const auto& target) { replaceText(target / "screenings.txt", "2099-10-12 19:00:00", "invalid"); });
    rejected([](const auto& target) { replaceText(target / "screenings.txt", "2099-10-12 19:00:00", "2099-02-31 19:00:00"); });
    rejected([](const auto& target) { replaceText(target / "screenings.txt", "20:45:00", "20:44:59"); });
    rejected([](const auto& target) { replaceText(target / "screenings.txt", "20:30:00", "20:31:00"); });
    rejected([](const auto& target) { replaceText(target / "screenings.txt", "|80000|", "|0|"); });
    rejected([](const auto& target) { replaceText(target / "auditoriums.txt", "|2|2|4|", "|2|2|5|"); });
    rejected([](const auto& target) { replaceText(target / "auditoriums.txt", "|Room|", "|   |"); });
    rejected([](const auto& target) { replaceText(target / "seats.txt", "A02|A|2|", "A02|A|1|"); });
    rejected([](const auto& target) { replaceText(target / "seats.txt", "B02|B|2|", "B02|Z|2|"); });
    rejected([](const auto& target) { replaceText(target / "films.txt", "|90|", "|0|"); });
    rejected([](const auto& target) { replaceText(target / "users.txt", "|MANAGER|", "|UNKNOWN|"); });
    rejected([](const auto& target) { replaceText(target / "users.txt", "|ACTIVE", "|BROKEN"); });
    rejected([](const auto& target) { replaceText(target / "films.txt", "film_id", "wrong_header"); });
    const auto newRoot = root / "new";
    ProjectDataEngine fresh(newRoot.string());
    assert(fresh.open(error));
    assert(fs::exists(newRoot / "users.txt") && fs::exists(newRoot / "tickets.txt"));
    assert(fresh.query<User>().isEmpty());
    const auto lostRoot = root / "all_lost";
    fs::create_directories(lostRoot);
    fs::copy_file(seed / "users.txt", lostRoot / "users.txt.bak");
    ProjectDataEngine allLost(lostRoot.string());
    assert(!allLost.open(error));
    assert(!fs::exists(lostRoot / "users.txt"));
    fs::remove_all(root);
}
