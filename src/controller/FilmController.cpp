#include "FilmController.h"

#include <exception>

namespace cinema
{
FilmController::FilmController(FilmService& service) : films(service) {}

bool FilmController::list(const std::string& text, std::optional<RecordStatus> status,
                          DoublyLinkedList<Film>& result, std::string& error) const
{
    error.clear();
    result.clear();
    try { result = films.list(text, status); return true; }
    catch (const std::exception& exception) { result.clear(); error = exception.what(); return false; }
}

bool FilmController::find(const std::string& filmId, Film& result, std::string& error) const
{
    error.clear();
    result = {};
    try
    {
        if (films.find(filmId, result)) return true;
        error = "Khong tim thay phim hoac phien dang nhap khong con hop le.";
        return false;
    }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool FilmController::create(const Film& film, std::string& error)
{
    error.clear();
    try { return films.create(film, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool FilmController::update(const Film& film, std::string& error)
{
    error.clear();
    try { return films.update(film, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}
}
