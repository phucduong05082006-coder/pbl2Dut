#ifndef CINEMA_CONTROLLER_FILM_CONTROLLER_H
#define CINEMA_CONTROLLER_FILM_CONTROLLER_H

#include "../service/FilmService.h"

namespace cinema
{
class FilmController final
{
public:
    explicit FilmController(FilmService& films);

    bool list(const std::string& text, std::optional<RecordStatus> status,
              DoublyLinkedList<Film>& result, std::string& error) const;
    bool find(const std::string& filmId, Film& result, std::string& error) const;
    bool create(const Film& film, std::string& error);
    bool update(const Film& film, std::string& error);

private:
    FilmService& films;
};
}

#endif
