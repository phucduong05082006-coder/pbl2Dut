#ifndef CINEMA_CONTROLLER_OPERATIONS_CONTROLLER_H
#define CINEMA_CONTROLLER_OPERATIONS_CONTROLLER_H

#include "../service/ScreeningService.h"
#include "../service/SaleService.h"
#include "../service/ReportService.h"

namespace cinema
{
class OperationsController final
{
public:
    OperationsController(ScreeningService& shows, SaleService& sales, ReportService& reports);
    bool screenings(const std::string& filmId, const std::string& roomId, const std::string& date,
        std::optional<ScreeningStatus> status, bool saleOnly, DoublyLinkedList<ScreeningDetails>& result, std::string& error) const;
    bool createScreening(const CreateScreeningRequest& request, Screening& result, std::string& error);
    bool changeStatus(const std::string& id, ScreeningStatus status, std::string& error);
    bool seatMap(const std::string& id, Matrix2D& result, std::string& error) const;
    bool preview(const SaleRequest& request, SaleDetails& result, std::string& error) const;
    bool sell(const SaleRequest& request, SaleReceipt& result, std::string& error);
    bool lookup(const std::string& id, bool byTicket, SaleDetails& result, std::string& error) const;
    bool revenue(const RevenueQuery& query, RevenueReport& result, std::string& error) const;
    bool occupancy(const std::string& id, OccupancyReport& result, std::string& error) const;

private:
    ScreeningService& shows;
    SaleService& sales;
    ReportService& reports;
};
}

#endif
