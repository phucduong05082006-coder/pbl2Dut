#include <cassert>
#include <filesystem>
#include <fstream>

#include "repository/ProjectDataEngine.h"

namespace
{
void writeSeedData(const std::filesystem::path& root)
{
    std::filesystem::create_directories(root);
    std::ofstream(root / "users.txt")
        << "user_id|username|password_value|display_name|role|can_sell_ticket|status\n"
        << "U001|manager|demo|Quan ly|MANAGER|1|ACTIVE\n";
    std::ofstream(root / "films.txt")
        << "film_id|title|duration_minutes|age_rating|status\n"
        << "F001|Avengers Endgame|181|T13|ACTIVE\n"
        << "F002|Avatar|162|T13|ACTIVE\n"
        << "F003|Dune|155|T13|ACTIVE\n"
        << "F004|Hidden Echo|162|T13|INACTIVE\n"
        << "F005|Arrival|162|T13|ACTIVE\n";
    std::ofstream(root / "auditoriums.txt")
        << "auditorium_id|name|rows|columns|capacity|status\n"
        << "R001|Phong 01|1|1|1|ACTIVE\n";
    std::ofstream(root / "seats.txt")
        << "seat_id|auditorium_id|seat_code|row_label|seat_number|seat_type|status\n"
        << "S001|R001|A01|A|1|STANDARD|ACTIVE\n";
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
    const std::filesystem::path root = std::filesystem::temp_directory_path() / "cms_project_data_engine_check";
    std::filesystem::remove_all(root);
    writeSeedData(root);

    cinema::ProjectDataEngine engine(root.string());
    std::string error;
    assert(engine.open(error));
    assert(error.empty());

    cinema::SearchRequest request;
    request.entity = cinema::SearchEntity::Film;
    request.text = "ave";
    request.mode = cinema::SearchMode::Prefix;
    cinema::SearchResult result = engine.search(request);
    assert(result.size() == 1);
    assert(result.hits.getAt(0)->recordId == "F001");

    request.text = "ger";
    request.mode = cinema::SearchMode::Contains;
    result = engine.search(request);
    assert(result.size() == 1);

    cinema::Film film;
    assert(engine.getFilm("F002", film));
    assert(film.title == "Avatar");
    assert(!engine.getFilm("UNKNOWN", film));

    const cinema::ProjectDataEngine& reader = engine;
    auto allFilms = reader.query<cinema::Film>();
    assert(allFilms.size() == 5);
    assert(allFilms.getAt(0)->filmId == "F001");
    assert(allFilms.getAt(4)->filmId == "F005");

    const int minimumDuration = 160;
    cinema::QuerySpec<cinema::Film> specification;
    specification.where = [minimumDuration](const cinema::Film& value) {
        return value.status == cinema::RecordStatus::Active &&
               value.durationMinutes >= minimumDuration && value.ageRating == "T13";
    };
    auto filtered = reader.query(specification);
    assert(filtered.size() == 3);
    assert(filtered.getAt(0)->filmId == "F001");
    assert(filtered.getAt(1)->filmId == "F002");
    assert(filtered.getAt(2)->filmId == "F005");

    specification.orderBy = [](const cinema::Film& left, const cinema::Film& right) {
        return left.durationMinutes < right.durationMinutes;
    };
    auto ascending = reader.query(specification);
    assert(ascending.size() == 3);
    assert(ascending.getAt(0)->filmId == "F002");
    assert(ascending.getAt(1)->filmId == "F005");
    assert(ascending.getAt(2)->filmId == "F001");

    specification.limit = 1;
    auto limited = reader.query(specification);
    assert(limited.size() == 1);
    assert(limited.getAt(0)->filmId == "F002");

    specification.orderBy = [](const cinema::Film& left, const cinema::Film& right) {
        return left.durationMinutes > right.durationMinutes;
    };
    specification.limit = 20;
    auto descending = reader.query(specification);
    assert(descending.size() == 3);
    assert(descending.getAt(0)->filmId == "F001");
    assert(descending.getAt(1)->filmId == "F002");
    assert(descending.getAt(2)->filmId == "F005");

    limited.getAt(0)->title = "Changed snapshot";
    limited.clear();
    assert(engine.getFilm("F002", film));
    assert(film.title == "Avatar");
    assert(reader.query<cinema::Film>().getAt(0)->filmId == "F001");

    specification.where = [](const cinema::Film&) { return false; };
    assert(reader.query(specification).isEmpty());

    specification = {};
    specification.limit = 2;
    auto firstTwo = reader.query(specification);
    assert(firstTwo.size() == 2);
    assert(firstTwo.getAt(0)->filmId == "F001");
    assert(firstTwo.getAt(1)->filmId == "F002");

    specification.limit = -1;
    bool rejected = false;
    try
    {
        reader.query(specification);
    }
    catch (const std::invalid_argument&)
    {
        rejected = true;
    }
    assert(rejected);

    assert(reader.query<cinema::User>().getAt(0)->userId == "U001");
    assert(reader.query<cinema::Auditorium>().getAt(0)->auditoriumId == "R001");
    assert(reader.query<cinema::Seat>().getAt(0)->seatId == "S001");
    assert(reader.query<cinema::Screening>().isEmpty());
    assert(reader.query<cinema::SaleTransaction>().isEmpty());
    assert(reader.query<cinema::Ticket>().isEmpty());

    cinema::AggregateSpec<cinema::Film> aggregation;
    aggregation.groupBy = [](const cinema::Film& value) {
        return value.status == cinema::RecordStatus::Active ? std::string("active") : std::string("inactive");
    };
    aggregation.value = [](const cinema::Film& value) { return static_cast<long long>(value.durationMinutes); };
    auto groups = reader.aggregate(aggregation);
    assert(groups.size() == 2);
    assert(groups.getAt(0)->key == "active" && groups.getAt(0)->count == 4 && groups.getAt(0)->total == 660);
    assert(groups.getAt(1)->key == "inactive" && groups.getAt(1)->count == 1 && groups.getAt(1)->total == 162);
    groups.getAt(0)->total = 0;
    assert(reader.aggregate(aggregation).getAt(0)->total == 660);

    aggregation = {};
    aggregation.value = [](const cinema::Film& value) { return static_cast<long long>(value.durationMinutes); };
    auto total = reader.aggregate(aggregation);
    assert(total.size() == 1 && total.getAt(0)->count == 5 && total.getAt(0)->total == 822);
    aggregation.where = [](const cinema::Film&) { return false; };
    assert(reader.aggregate(aggregation).isEmpty());

    aggregation = {};
    aggregation.value = [](const cinema::Film&) { return std::numeric_limits<long long>::max(); };
    bool overflowRejected = false;
    try { reader.aggregate(aggregation); }
    catch (const std::overflow_error&) { overflowRejected = true; }
    assert(overflowRejected);
    aggregation.value = [](const cinema::Film&) { return std::numeric_limits<long long>::min(); };
    overflowRejected = false;
    try { reader.aggregate(aggregation); }
    catch (const std::overflow_error&) { overflowRejected = true; }
    assert(overflowRejected);

    aggregation = {};
    aggregation.groupBy = [](const cinema::Film&) { return std::string(); };
    bool invalidGroupRejected = false;
    try { reader.aggregate(aggregation); }
    catch (const std::invalid_argument&) { invalidGroupRejected = true; }
    assert(invalidGroupRejected);

    assert(engine.save(error));
    cinema::ProjectDataEngine reloaded(root.string());
    assert(reloaded.open(error));
    assert(reloaded.query<cinema::Film>().size() == 5);
    assert(reloaded.getFilm("F002", film));
    assert(film.title == "Avatar");

    std::filesystem::remove_all(root);
    return 0;
}
