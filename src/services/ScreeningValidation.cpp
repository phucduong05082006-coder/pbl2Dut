#include "services/ScreeningValidation.h"

#include "services/RepositoryLookup.h"
#include "services/ScreeningTime.h"

#include <stdexcept>

void service_validation::screening(const Screening& candidate, const FilmRepository& films,
                                   const AuditoriumRepository& rooms,
                                   const ScreeningRepository& screenings) {
    service_detail::require(rooms, candidate.auditoriumId());
    const auto start = screening_time::minutes(candidate.startTime());
    const auto end = screening_time::endMinute(candidate, films, 15);
    screenings.forEach([&](const Screening& item) {
        if (item.id() != candidate.id() && item.auditoriumId() == candidate.auditoriumId()
            && start < screening_time::endMinute(item, films, 15)
            && screening_time::minutes(item.startTime()) < end) {
            throw std::invalid_argument("Screening overlaps another room booking");
        }
    });
}
