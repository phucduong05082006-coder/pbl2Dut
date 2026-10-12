#include <cassert>
#include "service/AuthService.h"
#include "service/FilmService.h"
#include "service/AuditoriumService.h"
#include "service/ScreeningService.h"
#include <chrono>
#include <filesystem>

#include "controller/AuthController.h"
#include "controller/FilmController.h"

int main()
{
    using namespace cinema;
    const auto root = std::filesystem::temp_directory_path() /
        ("cms_film_controller_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    ProjectDataEngine engine(root.string());
    std::string error;
    assert(engine.open(error));
    AuthService authService(engine);
    AuthController auth(authService);
    FilmService filmService(engine, authService);
    FilmController films(filmService);
    assert(auth.initializeManager("manager", "Manager", "secret", error));
    LoginSession session;
    assert(auth.login("manager", "secret", session, error));

    DoublyLinkedList<Film> result;
    assert(films.list("", std::nullopt, result, error) && result.isEmpty());
    assert(!films.create({"", "Tau", 98, "P", RecordStatus::Active}, error));
    assert(!films.create({"F001", "", 98, "P", RecordStatus::Active}, error));
    assert(!films.create({"F001", "Tau tren song", 0, "P", RecordStatus::Active}, error));
    assert(!films.create({"F001", " \t ", 98, "P", RecordStatus::Active}, error));
    assert(!films.create({"F001", "Tau", 98, " ", RecordStatus::Active}, error));
    assert(!films.create({"F001", "Bad|title", 98, "P", RecordStatus::Active}, error));
    assert(!films.create({"F001", "Tau", 98, "P", static_cast<RecordStatus>(42)}, error));
    assert(films.create({"F001", "Tau tren song", 98, "P", RecordStatus::Active}, error));
    assert(!films.create({"F001", "Trung ma", 100, "P", RecordStatus::Active}, error));
    assert(films.create({"F002", "Bau troi xanh", 110, "T13", RecordStatus::Inactive}, error));
    assert(films.list("tau", RecordStatus::Active, result, error) && result.size() == 1);
    assert(result.getHead()->data.filmId == "F001");
    assert(films.list("", RecordStatus::Inactive, result, error) && result.size() == 1);
    assert(films.list("F00", std::nullopt, result, error) && result.size() == 2);
    assert(result.getHead()->data.filmId == "F001");
    assert(!films.list("", static_cast<RecordStatus>(42), result, error) && result.isEmpty());
    Film film;
    assert(films.find("F001", film, error) && film.title == "Tau tren song");
    assert(films.update({"F001", "Tau tren song moi", 99, "P", RecordStatus::Active}, error));
    assert(films.find("F001", film, error) && film.durationMinutes == 99);
    assert(!films.update({"F003", "Khong ton tai", 90, "P", RecordStatus::Active}, error));
    std::filesystem::create_directory(root / "tickets.txt.tmp");
    film.title = "Khong duoc luu";
    assert(!films.update(film, error));
    assert(films.find("F001", film, error) && film.title == "Tau tren song moi");
    std::filesystem::remove(root / "tickets.txt.tmp");
    ProjectDataEngine reopened(root.string());
    assert(reopened.open(error));
    assert(reopened.getFilm("F001", film) && film.title == "Tau tren song moi");

    AuditoriumService rooms(engine, authService);
    Auditorium room{"R001", "Room", 1, 1, 1, RecordStatus::Active};
    DoublyLinkedList<Seat> seats;
    seats.addLast({"S001", "R001", "A01", "A", 1, SeatType::Standard, RecordStatus::Active});
    assert(rooms.save(room, seats, error));
    ScreeningService screenings(engine, authService, [] { return "2099-01-01 00:00:00"; });
    Screening screening;
    assert(screenings.create({"SC001", "F001", "R001", "2099-01-02 10:00:00", 80000, true}, screening, error));
    film.durationMinutes = 100;
    assert(!films.update(film, error));
    film.durationMinutes = 99;
    film.status = RecordStatus::Inactive;
    assert(!films.update(film, error));
    assert(films.find("F001", film, error) && film.status == RecordStatus::Active);
    auth.logout();
    assert(!films.list("", std::nullopt, result, error));
    assert(result.isEmpty() && !error.empty());
    assert(!films.find("F001", film, error) && film.filmId.empty());
    assert(!films.create({"F003", "Bi chan", 90, "P", RecordStatus::Active}, error));
    assert(auth.login("manager", "secret", session, error));
    User manager;
    assert(engine.getUser("U001", manager));
    manager.role = UserRole::Seller;
    assert(engine.saveUser(manager, error));
    assert(!films.update({"F001", "Bi chan", 90, "P", RecordStatus::Active}, error));
    std::filesystem::remove_all(root);
}
