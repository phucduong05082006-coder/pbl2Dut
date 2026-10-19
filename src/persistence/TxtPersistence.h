#pragma once

#include "repositories/AuditoriumRepository.h"
#include "repositories/FilmRepository.h"
#include "repositories/ScreeningRepository.h"
#include "repositories/SeatRepository.h"
#include "repositories/TicketRepository.h"

#include <charconv>
#include <filesystem>
#include <fstream>
#include <locale>
#include <sstream>

namespace txt_persistence {

inline void validateUtf8(const std::string& text) {
    for (std::size_t i = 0; i < text.size();) {
        const auto first = static_cast<unsigned char>(text[i++]);
        if (first < 0x80) {
            continue;
        }
        const int remaining = first >= 0xc2 && first <= 0xdf ? 1
            : first >= 0xe0 && first <= 0xef ? 2
            : first >= 0xf0 && first <= 0xf4 ? 3 : -1;
        if (remaining < 0 || text.size() - i < static_cast<std::size_t>(remaining)) {
            throw std::invalid_argument("Invalid UTF-8");
        }
        std::uint32_t code = first & ((1u << (6 - remaining)) - 1);
        for (int j = 0; j < remaining; ++j) {
            const auto byte = static_cast<unsigned char>(text[i++]);
            if ((byte & 0xc0) != 0x80) {
                throw std::invalid_argument("Invalid UTF-8 continuation");
            }
            code = (code << 6) | (byte & 0x3f);
        }
        if ((remaining == 1 && code < 0x80) || (remaining == 2 && code < 0x800)
            || (remaining == 3 && code < 0x10000) || code > 0x10ffff
            || (code >= 0xd800 && code <= 0xdfff)) {
            throw std::invalid_argument("Invalid UTF-8 code point");
        }
    }
}

inline std::int64_t number(const std::string& field) {
    std::int64_t result = 0;
    const auto parsed = std::from_chars(field.data(), field.data() + field.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != field.data() + field.size()) {
        throw std::invalid_argument("Expected a complete int64 field");
    }
    return result;
}

inline const char* header(const Film*) { return "film_id|title|duration_minutes"; }
inline const char* header(const Auditorium*) { return "auditorium_id|name|row_count|seats_per_row"; }
inline const char* header(const Seat*) { return "seat_id|auditorium_id|row_number|seat_number"; }
inline const char* header(const Screening*) { return "screening_id|film_id|auditorium_id|start_time|ticket_price"; }
inline const char* header(const Ticket*) { return "ticket_id|screening_id|seat_id|sold_at|unit_price"; }

inline Film parse(const std::string* f, const Film*) {
    return Film(number(f[0]), f[1], number(f[2]));
}
inline Auditorium parse(const std::string* f, const Auditorium*) {
    return Auditorium(number(f[0]), f[1], number(f[2]), number(f[3]));
}
inline Seat parse(const std::string* f, const Seat*) {
    return Seat(number(f[0]), number(f[1]), number(f[2]), number(f[3]));
}
inline Screening parse(const std::string* f, const Screening*) {
    return Screening(number(f[0]), number(f[1]), number(f[2]), f[3], number(f[4]));
}
inline Ticket parse(const std::string* f, const Ticket*) {
    return Ticket(number(f[0]), number(f[1]), number(f[2]), f[3], number(f[4]));
}

inline void writeRecord(std::ostream& out, const Film& value) {
    out << value.id() << '|' << value.title() << '|' << value.durationMinutes();
}
inline void writeRecord(std::ostream& out, const Auditorium& value) {
    out << value.id() << '|' << value.name() << '|' << value.rowCount() << '|' << value.seatsPerRow();
}
inline void writeRecord(std::ostream& out, const Seat& value) {
    out << value.id() << '|' << value.auditoriumId() << '|' << value.rowNumber() << '|' << value.seatNumber();
}
inline void writeRecord(std::ostream& out, const Screening& value) {
    out << value.id() << '|' << value.filmId() << '|' << value.auditoriumId()
        << '|' << value.startTime() << '|' << value.ticketPrice();
}
inline void writeRecord(std::ostream& out, const Ticket& value) {
    out << value.id() << '|' << value.screeningId() << '|' << value.seatId()
        << '|' << value.soldAt() << '|' << value.unitPrice();
}

template <typename Function>
void atLine(const std::filesystem::path& path, std::size_t line, Function action) {
    try {
        action();
    } catch (const std::bad_alloc&) {
        throw;
    } catch (const std::exception& error) {
        throw std::runtime_error(path.string() + ":" + std::to_string(line) + ": " + error.what());
    }
}

template <typename RepositoryType>
void load(const std::filesystem::path& path, RepositoryType& repository) {
    using T = typename RepositoryType::value_type;
    const T* tag = nullptr;
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error(path.string() + ": Cannot open for reading");
    }
    const std::string expected = header(tag);
    std::size_t fieldCount = 1;
    for (char character : expected) {
        fieldCount += character == '|';
    }
    std::size_t lineNumber = 0;
    std::string line;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (lineNumber == 1 && line.compare(0, 3, "\xef\xbb\xbf") == 0) {
            line.erase(0, 3);
        }
        atLine(path, lineNumber, [&] {
            validateUtf8(line);
            if (lineNumber == 1) {
                if (line != expected) {
                    throw std::invalid_argument("Incorrect header");
                }
                return;
            }
            std::string fields[5];
            std::size_t start = 0;
            for (std::size_t i = 0; i < fieldCount; ++i) {
                const auto end = line.find('|', start);
                if ((i + 1 == fieldCount) != (end == std::string::npos)) {
                    throw std::invalid_argument("Incorrect field count");
                }
                fields[i] = line.substr(start, end == std::string::npos ? end : end - start);
                start = end == std::string::npos ? line.size() : end + 1;
            }
            if (!repository.insert(parse(fields, tag))) {
                throw std::invalid_argument("Duplicate primary key");
            }
        });
    }
    if (input.bad() || !input.eof()) {
        throw std::runtime_error(path.string() + ": Read failed");
    }
    if (lineNumber == 0) {
        throw std::runtime_error(path.string() + ":1: Missing header");
    }
}

template <typename RepositoryType>
void save(const std::filesystem::path& path, const RepositoryType& repository) {
    using T = typename RepositoryType::value_type;
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error(path.string() + ": Cannot open for writing");
    }
    output << header(static_cast<const T*>(nullptr)) << '\n';
    std::size_t lineNumber = 1;
    repository.forEach([&](const T& value) {
        atLine(path, ++lineNumber, [&] {
            std::ostringstream row;
            row.exceptions(std::ios::badbit | std::ios::failbit);
            row.imbue(std::locale::classic());
            writeRecord(row, value);
            const auto text = row.str();
            validateUtf8(text);
            output << text << '\n';
            if (!output) {
                throw std::runtime_error("Write failed");
            }
        });
    });
    output.flush();
    if (!output) {
        throw std::runtime_error(path.string() + ": Flush failed");
    }
    output.close();
    if (output.fail()) {
        throw std::runtime_error(path.string() + ": Close failed");
    }
}

}
