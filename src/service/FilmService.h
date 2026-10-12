#ifndef CINEMA_SERVICE_FILM_SERVICE_H
#define CINEMA_SERVICE_FILM_SERVICE_H

#include <optional>
#include "AuthService.h"
#include "../entity/Film.h"

namespace cinema
{
class FilmService final
{
public:
    FilmService(ProjectDataEngine& dataEngine, AuthService& auth);

    bool save(const Film& film, std::string& error);
    bool create(const Film& film, std::string& error);
    bool update(const Film& film, std::string& error);
    DoublyLinkedList<Film> list(const std::string& text = {},
                               std::optional<RecordStatus> status = std::nullopt) const;
    bool find(const std::string& filmId, Film& film) const;
    DoublyLinkedList<Film> activeFilms() const;
    SearchResult search(const std::string& text) const;

private:
    ProjectDataEngine& dataEngine;
    AuthService& auth;
};
}

#endif
