#pragma once

#include "entities/EntityId.h"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace entity_detail {

inline std::int64_t positive(std::int64_t value, const char* field) {
    if (value <= 0) {
        throw std::invalid_argument(std::string(field) + " must be positive");
    }
    return value;
}

inline std::string name(std::string value) {
    for (unsigned char character : value) {
        if (character < 32 || character == 127 || character == '|') {
            throw std::invalid_argument("Name contains a control character or delimiter");
        }
    }
    const std::size_t first = value.find_first_not_of(' ');
    if (first == std::string::npos) {
        throw std::invalid_argument("Name must not be empty");
    }
    return value.substr(first, value.find_last_not_of(' ') - first + 1);
}

inline std::string dateTime(std::string value) {
    if (value.size() != 16 || value[4] != '-' || value[7] != '-'
        || value[10] != ' ' || value[13] != ':') {
        throw std::invalid_argument("Date must use YYYY-MM-DD HH:mm");
    }
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (index != 4 && index != 7 && index != 10 && index != 13
            && (value[index] < '0' || value[index] > '9')) {
            throw std::invalid_argument("Date contains a non-digit");
        }
    }
    const int year = std::stoi(value.substr(0, 4));
    const int month = std::stoi(value.substr(5, 2));
    const int day = std::stoi(value.substr(8, 2));
    const int hour = std::stoi(value.substr(11, 2));
    const int minute = std::stoi(value.substr(14, 2));
    if (year == 0 || month < 1 || month > 12 || hour > 23 || minute > 59) {
        throw std::invalid_argument("Date or time is out of range");
    }
    const int daysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const bool leapYear = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
    const int maximumDay = daysPerMonth[month - 1] + (month == 2 && leapYear ? 1 : 0);
    if (day < 1 || day > maximumDay) {
        throw std::invalid_argument("Day is out of range");
    }
    return value;
}

}
