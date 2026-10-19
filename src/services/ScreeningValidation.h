#pragma once

#include "entities/Screening.h"
#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"

namespace service_validation {

void screening(const Screening& candidate, const FilmRepository& films,
               const AuditoriumRepository& rooms, const ScreeningRepository& screenings);

}
