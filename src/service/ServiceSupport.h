#ifndef CINEMA_SERVICE_SERVICE_SUPPORT_H
#define CINEMA_SERVICE_SERVICE_SUPPORT_H

#include <functional>
#include <string>

namespace cinema
{
class AuthService;
class ProjectDataEngine;
using ServiceClock = std::function<std::string()>;

namespace detail
{
std::string currentTime(const ServiceClock& clock);
bool readTime(const ServiceClock& clock, std::string& text, std::string& error);
void requireRead(const AuthService& auth);
std::string newIdentifier(const char* prefix);
bool hasBookings(ProjectDataEngine& engine, const std::string& filmId, const std::string& roomId);
bool hasFutureBookings(ProjectDataEngine& engine, const std::string& filmId, const std::string& roomId);
}
}

#endif
