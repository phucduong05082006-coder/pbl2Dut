#include "ApplicationData.h"
#include "persistence/TxtPersistence.h"
#include "services/ScreeningValidation.h"

#include <exception>

namespace fs = std::filesystem;

namespace {
constexpr const char* files[] = {
    "films.txt", "auditoriums.txt", "seats.txt", "screenings.txt", "tickets.txt"
};

fs::file_status inspectPath(const fs::path& path) {
    std::error_code error;
    const auto result = fs::symlink_status(path, error);
    if (error && error != std::errc::no_such_file_or_directory) {
        throw fs::filesystem_error("Cannot inspect path", path, error);
    }
    return result;
}

int fileCount(const fs::path& directory) {
    if (!fs::is_directory(directory)) {
        throw std::runtime_error(directory.string() + ": Data directory must exist");
    }
    // Opening the iterator also checks that the directory can be read.
    const fs::directory_iterator readable(directory);
    (void)readable;
    int count = 0;
    for (const char* name : files) {
        const auto path = directory / name;
        const auto entry = inspectPath(path);
        if (fs::exists(entry)) {
            if (!fs::is_regular_file(entry)) {
                throw std::runtime_error(path.string() + ": Expected a regular file, not a symlink/directory");
            }
            ++count;
        }
    }
    return count;
}

template <typename RepositoryType, typename Function>
void checkRecords(const fs::path& path, const RepositoryType& repository, Function check) {
    std::size_t line = 1;
    repository.forEach([&](const auto& value) {
        txt_persistence::atLine(path, ++line, [&] { check(value); });
    });
}

void cleanOwnedStage(const fs::path& stage) {
    if (!fs::is_directory(inspectPath(stage))) {
        throw std::runtime_error("Owned staging directory is missing or replaced");
    }
    for (const char* subdirectory : {"new", "old"}) {
        const auto folder = stage / subdirectory;
        if (!fs::exists(inspectPath(folder))) {
            continue;
        }
        if (!fs::is_directory(inspectPath(folder))) {
            throw std::runtime_error("Staging subdirectory was replaced");
        }
        for (const char* name : files) {
            const auto path = folder / name;
            const auto entry = inspectPath(path);
            if (fs::exists(entry)) {
                if (!fs::is_regular_file(entry)) {
                    throw std::runtime_error("Unexpected staging entry; refusing to remove it");
                }
                fs::remove(path);
            }
        }
        fs::remove(folder); // Fails rather than deleting any unexpected contents.
    }
    fs::remove(stage);
}
}

ApplicationData::ApplicationData(fs::path directory)
    : directory_(fs::absolute(std::move(directory)).lexically_normal()) {}

bool ApplicationData::isDirty() const noexcept {
    return films.isDirty() || rooms.isDirty() || seats.isDirty()
        || screenings.isDirty() || tickets.isDirty();
}

void ApplicationData::markSaved() noexcept {
    films.markSaved();
    rooms.markSaved();
    seats.markSaved();
    screenings.markSaved();
    tickets.markSaved();
}

ApplicationData::LoadResult ApplicationData::load(bool confirmEmpty) {
    if (ready_ || isDirty() || !films.empty() || !rooms.empty() || !seats.empty()
        || !screenings.empty() || !tickets.empty()) {
        throw std::logic_error("Load requires a fresh ApplicationData instance");
    }
    const int count = fileCount(directory_);
    if (fs::exists(inspectPath(directory_ / ".cms-save"))) {
        throw std::runtime_error("Unfinished save in .cms-save; inspect/restore the complete backup before loading");
    }
    if (count == 0) {
        for (const auto& entry : fs::directory_iterator(directory_)) {
            if (entry.path().filename().string().find(".cms-backup-") == 0) {
                throw std::runtime_error("Data files missing but backups exist; restore instead of starting empty");
            }
        }
        if (!confirmEmpty) {
            return LoadResult::NeedsEmptyConfirmation;
        }
        ready_ = true;
        return LoadResult::Loaded;
    }
    if (count != 5) {
        for (const char* name : files) {
            if (!fs::exists(inspectPath(directory_ / name))) {
                throw std::runtime_error((directory_ / name).string() + ": Incomplete data set; file is missing");
            }
        }
    }
    ApplicationData staged(directory_);
    txt_persistence::load(directory_ / files[0], staged.films);
    txt_persistence::load(directory_ / files[1], staged.rooms);
    txt_persistence::load(directory_ / files[2], staged.seats);
    txt_persistence::load(directory_ / files[3], staged.screenings);
    txt_persistence::load(directory_ / files[4], staged.tickets);
    staged.validate();
    films.replaceWith(staged.films);
    rooms.replaceWith(staged.rooms);
    seats.replaceWith(staged.seats);
    screenings.replaceWith(staged.screenings);
    tickets.records_.replaceWith(staged.tickets.records_);
    markSaved();
    ready_ = hadFiles_ = true;
    return LoadResult::Loaded;
}

void ApplicationData::validate() const {
    checkRecords(directory_ / files[0], films, [](const Film& film) {
        Film(film.id(), film.title(), film.durationMinutes());
        txt_persistence::validateUtf8(film.title());
    });
    HashTable<std::string, bool> positions;
    HashTable<EntityId, std::size_t> seatCounts;
    checkRecords(directory_ / files[2], seats, [&](const Seat& seat) {
        Seat(seat.id(), seat.auditoriumId(), seat.rowNumber(), seat.seatNumber());
        const auto* room = rooms.findById(seat.auditoriumId());
        if (!room || seat.rowNumber() > room->rowCount() || seat.seatNumber() > room->seatsPerRow()) {
            throw std::invalid_argument("Seat references a missing room or an out-of-range position");
        }
        const auto key = std::to_string(seat.auditoriumId()) + ":"
            + std::to_string(seat.rowNumber()) + ":" + std::to_string(seat.seatNumber());
        if (!positions.insert(key, true)) {
            throw std::invalid_argument("Duplicate room-row-column");
        }
        auto* count = seatCounts.find(room->id());
        if (count) {
            ++*count;
        } else {
            seatCounts.insert(room->id(), 1);
        }
    });
    checkRecords(directory_ / files[1], rooms, [&](const Auditorium& room) {
        Auditorium(room.id(), room.name(), room.rowCount(), room.seatsPerRow());
        txt_persistence::validateUtf8(room.name());
        const auto* count = seatCounts.find(room.id());
        if (!count || *count != static_cast<std::uint64_t>(room.capacity())) {
            throw std::invalid_argument("Room does not have its complete rectangular seat layout");
        }
    });
    checkRecords(directory_ / files[3], screenings, [&](const Screening& screening) {
        Screening(screening.id(), screening.filmId(), screening.auditoriumId(),
                  screening.startTime(), screening.ticketPrice());
        service_validation::screening(screening, films, rooms, screenings);
    });
    HashTable<std::string, bool> soldSeats;
    checkRecords(directory_ / files[4], tickets, [&](const Ticket& ticket) {
        Ticket(ticket.id(), ticket.screeningId(), ticket.seatId(), ticket.soldAt(), ticket.unitPrice());
        const auto* screening = screenings.findById(ticket.screeningId());
        const auto* seat = seats.findById(ticket.seatId());
        if (!screening || !seat || seat->auditoriumId() != screening->auditoriumId()) {
            throw std::invalid_argument("Ticket references a missing screening/seat or the wrong room");
        }
        if (ticket.soldAt() >= screening->startTime()) {
            throw std::invalid_argument("Ticket must be sold before screening start");
        }
        const auto key = std::to_string(ticket.screeningId()) + ":" + std::to_string(ticket.seatId());
        if (!soldSeats.insert(key, true)) {
            throw std::invalid_argument("Duplicate screening-seat ticket");
        }
    });
}

void ApplicationData::fileStep(const char* phase, std::size_t index) const {
    if (beforeFileStep_) {
        beforeFileStep_(phase, index);
    }
}

void ApplicationData::retryRecovery() {
    if (!recoveryRequired_) {
        return;
    }
    const auto stage = directory_ / ".cms-save";
    if (!ownsStage_ || !fs::is_directory(inspectPath(stage))
        || !fs::is_directory(inspectPath(stage / "old"))
        || !fs::is_directory(inspectPath(stage / "new"))) {
        throw std::runtime_error("Recovery staging directory is missing or replaced");
    }
    for (std::size_t i = 0; i < 5; ++i) {
        fileStep("restore", i);
        const auto original = directory_ / files[i];
        if (hadFiles_) {
            const auto backup = stage / "old" / files[i];
            const auto temporary = stage / "new" / files[i];
            if (!fs::is_regular_file(inspectPath(backup))) {
                throw std::runtime_error(backup.string() + ": Recovery requires a complete backup");
            }
            fs::copy_file(backup, temporary, fs::copy_options::overwrite_existing);
            fs::rename(temporary, original);
        } else if (fs::exists(inspectPath(original))) {
            if (!fs::is_regular_file(inspectPath(original))) {
                throw std::runtime_error(original.string() + ": Refusing to remove an unexpected entry");
            }
            fs::remove(original);
        }
    }
    recoveryRequired_ = false;
}

void ApplicationData::save() {
    if (!ready_) {
        throw std::logic_error("Load and validate data before saving");
    }
    if (recoveryRequired_) {
        throw std::runtime_error("Previous rollback failed; preserve RAM and .cms-save for manual recovery");
    }
    validate();
    const int count = fileCount(directory_);
    if (count != (hadFiles_ ? 5 : 0)) {
        throw std::runtime_error("Data file set changed externally; refusing to overwrite it");
    }
    const auto stage = directory_ / ".cms-save";
    if (ownsStage_) {
        cleanOwnedStage(stage);
        ownsStage_ = false;
    }
    fs::path originals[5], temporary[5], backups[5];
    for (std::size_t i = 0; i < 5; ++i) {
        originals[i] = directory_ / files[i];
        temporary[i] = stage / "new" / files[i];
        backups[i] = stage / "old" / files[i];
    }
    fs::path archive;
    for (std::uint64_t suffix = 1;; ++suffix) {
        archive = directory_ / (".cms-backup-" + std::to_string(suffix));
        if (!fs::exists(inspectPath(archive))) {
            break;
        }
        if (suffix == std::numeric_limits<std::uint64_t>::max()) {
            throw std::overflow_error("Backup names exhausted");
        }
    }
    if (!fs::create_directory(stage)) {
        throw std::runtime_error("Existing .cms-save must be inspected before saving");
    }
    ownsStage_ = true;
    std::size_t installed = 0;
    try {
        fs::create_directory(stage / "new");
        fs::create_directory(stage / "old");
        fileStep("write", 0); txt_persistence::save(temporary[0], films);
        fileStep("write", 1); txt_persistence::save(temporary[1], rooms);
        fileStep("write", 2); txt_persistence::save(temporary[2], seats);
        fileStep("write", 3); txt_persistence::save(temporary[3], screenings);
        fileStep("write", 4); txt_persistence::save(temporary[4], tickets);
        if (hadFiles_) {
            for (std::size_t i = 0; i < 5; ++i) {
                fileStep("backup", i);
                fs::copy_file(originals[i], backups[i]);
            }
        }
        for (std::size_t i = 0; i < 5; ++i) {
            fileStep("install", i);
            fs::rename(temporary[i], originals[i]);
            ++installed;
        }
        fileStep("archive", 0);
        // Keep backups; an unfinished .cms-save blocks loading after interruption.
        fs::rename(stage, archive);
    } catch (...) {
        const auto originalError = std::current_exception();
        bool restored = true;
        for (std::size_t i = 0; i < installed; ++i) {
            try {
                fileStep("restore", i);
                if (hadFiles_) {
                    fs::copy_file(backups[i], temporary[i], fs::copy_options::overwrite_existing);
                    fs::rename(temporary[i], originals[i]);
                } else {
                    fs::remove(originals[i]);
                }
            } catch (...) {
                restored = false;
            }
        }
        recoveryRequired_ = !restored;
        try {
            std::rethrow_exception(originalError);
        } catch (const std::exception& error) {
            throw std::runtime_error(std::string("Save failed: ") + error.what()
                + (restored ? "; previous disk state restored; RAM and .cms-save retained"
                            : "; ROLLBACK FAILED; preserve RAM and .cms-save for manual recovery"));
        }
    }
    ownsStage_ = false;
    hadFiles_ = true;
    markSaved();
}
