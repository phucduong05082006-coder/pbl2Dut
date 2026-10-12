#include "FilmService.h"
#include "ServiceSupport.h"

namespace cinema
{
using namespace detail;

FilmService::FilmService(ProjectDataEngine& engine, AuthService& authentication) : dataEngine(engine), auth(authentication) {}

bool FilmService::save(const Film& film, std::string& error)
{
    if (!auth.requireManager(error)) return false;
    if (film.filmId.find_first_not_of(" \t\r\n\f\v") == std::string::npos ||
        film.title.find_first_not_of(" \t\r\n\f\v") == std::string::npos ||
        film.ageRating.find_first_not_of(" \t\r\n\f\v") == std::string::npos)
    { error = "Ma phim, ten phim va phan loai do tuoi khong duoc de trong."; return false; }
    Film existing;
    if (dataEngine.getFilm(film.filmId, existing))
    {
        if (film.durationMinutes != existing.durationMinutes && hasBookings(dataEngine, film.filmId, {}))
        { error = "Khong sua thoi luong phim da co lich chieu."; return false; }
        if (film.status == RecordStatus::Inactive && hasFutureBookings(dataEngine, film.filmId, {}))
        { error = "Phim con lich chieu chua ket thuc."; return false; }
    }
    return dataEngine.saveFilm(film, error);
}

bool FilmService::create(const Film& film, std::string& error)
{
    if (!auth.requireManager(error)) return false;
    Film existing;
    if (dataEngine.getFilm(film.filmId, existing))
    { error = "Ma phim da ton tai."; return false; }
    return save(film, error);
}

bool FilmService::update(const Film& film, std::string& error)
{
    if (!auth.requireManager(error)) return false;
    Film existing;
    if (!dataEngine.getFilm(film.filmId, existing))
    { error = "Khong tim thay phim can cap nhat."; return false; }
    return save(film, error);
}

DoublyLinkedList<Film> FilmService::list(const std::string& text, std::optional<RecordStatus> status) const
{
    requireRead(auth);
    if (status && *status != RecordStatus::Active && *status != RecordStatus::Inactive)
        throw std::invalid_argument("Trang thai phim khong hop le.");
    HashTable<bool> matches;
    if (!text.empty())
    {
        const auto result = search(text);
        for (const Node<SearchHit>* node = result.hits.getHead(); node != nullptr; node = node->next)
            matches.insert(node->data.recordId, true);
    }
    QuerySpec<Film> specification;
    specification.where = [&](const Film& film) {
        return (!status || film.status == *status) && (text.empty() || matches.contains(film.filmId));
    };
    specification.orderBy = [](const Film& left, const Film& right) { return left.filmId < right.filmId; };
    return dataEngine.query(specification);
}

bool FilmService::find(const std::string& filmId, Film& film) const
{
    film = {};
    std::string error;
    return auth.requireLogin(error) && dataEngine.getFilm(filmId, film);
}

DoublyLinkedList<Film> FilmService::activeFilms() const
{
    requireRead(auth);
    QuerySpec<Film> specification;
    specification.where = [](const Film& film) { return film.status == RecordStatus::Active; };
    return dataEngine.query(specification);
}

SearchResult FilmService::search(const std::string& text) const
{
    requireRead(auth);
    return dataEngine.search({SearchEntity::Film, text, SearchMode::Contains, 0});
}
}
