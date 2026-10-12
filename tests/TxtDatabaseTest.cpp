#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

#include "repository/txt/TxtDatabase.h"

namespace
{
void writeSeedData(const std::filesystem::path& root)
{
    std::filesystem::create_directories(root);
    std::ofstream users(root / "users.txt");
    users << "user_id|username|password_value|display_name|role|can_sell_ticket|status\n"
          << "U001|manager|demo|Quan ly|MANAGER|1|ACTIVE\n";
    std::ofstream films(root / "films.txt");
    films << "film_id|title|duration_minutes|age_rating|status\n"
          << "F001|Chuyen tau mua thu|98|P|ACTIVE\n";
    std::ofstream rooms(root / "auditoriums.txt");
    rooms << "auditorium_id|name|rows|columns|capacity|status\n"
          << "R001|Phong 01|2|3|6|ACTIVE\n";
    std::ofstream seats(root / "seats.txt");
    seats << "seat_id|auditorium_id|seat_code|row_label|seat_number|seat_type|status\n"
          << "S001|R001|A01|A|1|STANDARD|ACTIVE\n"
          << "S002|R001|A02|A|2|STANDARD|ACTIVE\n"
          << "S003|R001|A03|A|3|STANDARD|ACTIVE\n"
          << "S004|R001|B01|B|1|STANDARD|ACTIVE\n"
          << "S005|R001|B02|B|2|STANDARD|ACTIVE\n"
          << "S006|R001|B03|B|3|STANDARD|ACTIVE\n";
    std::ofstream screenings(root / "screenings.txt");
    screenings << "screening_id|film_id|auditorium_id|start_time|end_time|room_available_until|ticket_price|status\n"
               << "SC001|F001|R001|2026-10-12 19:00:00|2026-10-12 20:38:00|2026-10-12 20:53:00|80000|OPEN\n";
    std::ofstream transactions(root / "transactions.txt");
    transactions << "transaction_id|screening_id|created_at|created_by|total_amount|payment_status|status\n";
    std::ofstream tickets(root / "tickets.txt");
    tickets << "ticket_id|transaction_id|screening_id|seat_id|unit_price|status\n";
}
}

int main()
{
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "cms_txt_database_check";
    std::filesystem::remove_all(root);
    writeSeedData(root);

    cinema::TxtDatabase database(root.string());
    std::string error;
    assert(database.load(error));
    assert(error.empty());
    assert(database.films().size() == 1);
    assert(database.findUserByUsername("manager") != nullptr);
    assert(database.findScreening("SC001") != nullptr);

    Matrix2D emptyMap = database.seatMapForScreening("SC001");
    assert(emptyMap.count('O') == 6);
    assert(emptyMap.count('X') == 0);

    cinema::SaleTransaction transaction{"TX001", "SC001", "2026-10-11 10:00:00", "U001", 160000, cinema::PaymentStatus::Confirmed, cinema::TransactionStatus::Completed};
    DoublyLinkedList<cinema::Ticket> saleTickets;
    saleTickets.addLast({"T001", "TX001", "SC001", "S001", 80000, cinema::TicketStatus::Issued});
    saleTickets.addLast({"T002", "TX001", "SC001", "S002", 80000, cinema::TicketStatus::Issued});
    assert(database.commitSale(transaction, saleTickets, error));
    assert(database.tickets().size() == 2);

    Matrix2D soldMap = database.seatMapForScreening("SC001");
    assert(soldMap.count('O') == 4);
    assert(soldMap.count('X') == 2);
    assert(std::filesystem::exists(root / "sale_journal.txt"));

    cinema::TxtDatabase reloaded(root.string());
    assert(reloaded.load(error));
    assert(reloaded.transactions().size() == 1);
    assert(reloaded.tickets().size() == 2);
    assert(reloaded.seatMapForScreening("SC001").count('X') == 2);

    std::filesystem::remove_all(root);
    return 0;
}
