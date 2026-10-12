#ifndef CINEMA_SERVICE_SCREENING_SERVICE_H
#define CINEMA_SERVICE_SCREENING_SERVICE_H

#include <optional>
#include "AuthService.h"
#include "ServiceSupport.h"
#include "../entity/Film.h"
#include "../entity/Auditorium.h"
#include "../entity/Screening.h"

namespace cinema
{
struct CreateScreeningRequest
{
    std::string screeningId;
    std::string filmId;
    std::string auditoriumId;
    std::string startTime;
    long long ticketPrice = 0;
    bool openForSale = true;
};

struct ScreeningDetails
{
    Screening screening;
    Film film;
    Auditorium auditorium;
};

class ScreeningService final
{
public:
    ScreeningService(ProjectDataEngine& dataEngine, AuthService& auth, ServiceClock clock = {});

    bool create(const CreateScreeningRequest& request,
                Screening& screening,
                std::string& error);
    bool changeStatus(const std::string& screeningId,
                      ScreeningStatus status,
                      std::string& error);
    DoublyLinkedList<ScreeningDetails> list(const std::string& filmId = {}, const std::string& roomId = {},
        const std::string& date = {}, std::optional<ScreeningStatus> status = std::nullopt, bool saleOnly = false) const;
    DoublyLinkedList<Screening> openScreenings() const;
    DoublyLinkedList<Screening> openScreeningsFor(
        const std::string& filmId,
        const std::string& auditoriumId,
        const std::string& date = {}) const;

private:
    ProjectDataEngine& dataEngine;
    AuthService& auth;
    ServiceClock clock;
};
}

#endif
