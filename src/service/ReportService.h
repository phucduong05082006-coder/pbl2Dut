#ifndef CINEMA_SERVICE_REPORT_SERVICE_H
#define CINEMA_SERVICE_REPORT_SERVICE_H

#include "AuthService.h"

namespace cinema
{
struct RevenueQuery
{
    std::string fromCreatedAt;
    std::string toCreatedAt;
    std::string filmId;
    std::string screeningId;
};

struct RevenueRow
{
    std::string filmId;
    std::string title;
    long long ticketCount = 0;
    long long revenue = 0;
};

struct RevenueReport
{
    DoublyLinkedList<RevenueRow> rows;
    long long ticketCount = 0;
    long long revenue = 0;
};

struct OccupancyReport
{
    int soldSeats = 0;
    int availableSeats = 0;
    double percent = 0;
};

class ReportService final
{
public:
    ReportService(ProjectDataEngine& dataEngine, AuthService& auth);

    bool revenueByFilm(const RevenueQuery& query,
                       RevenueReport& report,
                       std::string& error) const;
    bool occupancy(const std::string& screeningId, OccupancyReport& report, std::string& error) const;

private:
    ProjectDataEngine& dataEngine;
    AuthService& auth;
};
}

#endif
