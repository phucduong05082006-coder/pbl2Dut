#include "OperationsController.h"

#include <exception>

namespace cinema
{
OperationsController::OperationsController(ScreeningService& screeningService, SaleService& saleService, ReportService& reportService)
    : shows(screeningService), sales(saleService), reports(reportService) {}

bool OperationsController::screenings(const std::string& filmId, const std::string& roomId, const std::string& date,
    std::optional<ScreeningStatus> status, bool saleOnly, DoublyLinkedList<ScreeningDetails>& result, std::string& error) const
{
    error.clear();
    result.clear();
    try { result = shows.list(filmId, roomId, date, status, saleOnly); return true; }
    catch (const std::exception& exception) { result.clear(); error = exception.what(); return false; }
}

bool OperationsController::createScreening(const CreateScreeningRequest& request, Screening& result, std::string& error)
{
    error.clear();
    result = {};
    try { return shows.create(request, result, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool OperationsController::changeStatus(const std::string& id, ScreeningStatus status, std::string& error)
{
    error.clear();
    try { return shows.changeStatus(id, status, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool OperationsController::seatMap(const std::string& id, Matrix2D& result, std::string& error) const
{
    error.clear();
    result = {};
    try { result = sales.seatMap(id); return true; }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool OperationsController::preview(const SaleRequest& request, SaleDetails& result, std::string& error) const
{
    error.clear();
    result = {};
    try
    {
        SaleReceipt receipt;
        return sales.preview(request, receipt, error) && sales.describe(receipt, result, error);
    }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool OperationsController::sell(const SaleRequest& request, SaleReceipt& result, std::string& error)
{
    error.clear();
    result = {};
    try { return sales.sell(request, result, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool OperationsController::lookup(const std::string& id, bool byTicket, SaleDetails& result, std::string& error) const
{
    error.clear();
    result = {};
    try { return sales.lookup(id, byTicket, result, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool OperationsController::revenue(const RevenueQuery& query, RevenueReport& result, std::string& error) const
{
    error.clear();
    result = {};
    try { return reports.revenueByFilm(query, result, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}

bool OperationsController::occupancy(const std::string& id, OccupancyReport& result, std::string& error) const
{
    error.clear();
    result = {};
    try { return reports.occupancy(id, result, error); }
    catch (const std::exception& exception) { error = exception.what(); return false; }
}
}
