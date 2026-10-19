#pragma once

#include <string>

using ServiceClock = std::string (*)();

std::string localMinuteNow();
