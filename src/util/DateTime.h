#ifndef CINEMA_UTIL_DATE_TIME_H
#define CINEMA_UTIL_DATE_TIME_H

#include <ctime>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace cinema
{
inline std::string formatTimestamp(const std::tm& calendar)
{
    std::ostringstream output;
    output << std::put_time(&calendar, "%Y-%m-%d %H:%M:%S");
    return output.str();
}

inline bool parseTimestamp(const std::string& text, std::time_t& epoch)
{
    if (text.size() != 19) return false;
    std::tm calendar{};
    std::istringstream input(text);
    input >> std::get_time(&calendar, "%Y-%m-%d %H:%M:%S");
    if (input.fail()) return false;
    calendar.tm_isdst = -1;
    epoch = std::mktime(&calendar);
    const std::tm* normalized = epoch == std::time_t(-1) ? nullptr : std::localtime(&epoch);
    return normalized != nullptr && formatTimestamp(*normalized) == text;
}

inline bool calculateTimes(const std::string& start, int duration, std::string& end, std::string& available)
{
    std::time_t epoch;
    if (duration <= 0 || !parseTimestamp(start, epoch)) return false;
    const long long seconds = (static_cast<long long>(duration) + 15) * 60;
    if (epoch > std::numeric_limits<std::time_t>::max() - seconds) return false;
    const std::time_t endEpoch = epoch + static_cast<long long>(duration) * 60;
    const std::time_t availableEpoch = epoch + seconds;
    const std::tm* calendar = std::localtime(&endEpoch);
    if (calendar == nullptr) return false;
    end = formatTimestamp(*calendar);
    calendar = std::localtime(&availableEpoch);
    if (calendar == nullptr) return false;
    available = formatTimestamp(*calendar);
    return end.size() == 19 && available.size() == 19;
}
}

#endif
