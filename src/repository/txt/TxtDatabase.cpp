#include "TxtDatabase.h"
#include "../../util/DateTime.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>

namespace
{
using cinema::Auditorium;
using cinema::Film;
using cinema::RecordStatus;
using cinema::SaleTransaction;
using cinema::Screening;
using cinema::ScreeningStatus;
using cinema::Seat;
using cinema::SeatType;
using cinema::Ticket;
using cinema::TicketStatus;
using cinema::TransactionStatus;
using cinema::PaymentStatus;
using cinema::User;
using cinema::UserRole;

const char* const HEADER_USERS = "user_id|username|password_value|display_name|role|can_sell_ticket|status";
const char* const HEADER_FILMS = "film_id|title|duration_minutes|age_rating|status";
const char* const HEADER_AUDITORIUMS = "auditorium_id|name|rows|columns|capacity|status";
const char* const HEADER_SEATS = "seat_id|auditorium_id|seat_code|row_label|seat_number|seat_type|status";
const char* const HEADER_SCREENINGS = "screening_id|film_id|auditorium_id|start_time|end_time|room_available_until|ticket_price|status";
const char* const HEADER_TRANSACTIONS = "transaction_id|screening_id|created_at|created_by|total_amount|payment_status|status";
const char* const HEADER_TICKETS = "ticket_id|transaction_id|screening_id|seat_id|unit_price|status";
const char* const DATA_FILES[] = {"users.txt", "films.txt", "auditoriums.txt", "seats.txt", "screenings.txt", "transactions.txt", "tickets.txt"};

bool parseInt(const std::string& text, int& value)
{
    try
    {
        std::size_t consumed = 0;
        value = std::stoi(text, &consumed);
        return consumed == text.size();
    }
    catch (...)
    {
        return false;
    }
}

bool parseMoney(const std::string& text, long long& value)
{
    try
    {
        std::size_t consumed = 0;
        value = std::stoll(text, &consumed);
        return consumed == text.size();
    }
    catch (...)
    {
        return false;
    }
}

bool isBoolean(const std::string& text, bool& value)
{
    if (text == "1") { value = true; return true; }
    if (text == "0") { value = false; return true; }
    return false;
}

bool parseUserRole(const std::string& text, UserRole& value)
{
    if (text == "MANAGER") { value = UserRole::Manager; return true; }
    if (text == "SELLER") { value = UserRole::Seller; return true; }
    return false;
}

bool parseRecordStatus(const std::string& text, RecordStatus& value)
{
    if (text == "ACTIVE") { value = RecordStatus::Active; return true; }
    if (text == "INACTIVE") { value = RecordStatus::Inactive; return true; }
    return false;
}

bool parseScreeningStatus(const std::string& text, ScreeningStatus& value)
{
    if (text == "DRAFT") { value = ScreeningStatus::Draft; return true; }
    if (text == "OPEN") { value = ScreeningStatus::Open; return true; }
    if (text == "CLOSED") { value = ScreeningStatus::Closed; return true; }
    if (text == "COMPLETED") { value = ScreeningStatus::Completed; return true; }
    return false;
}

bool parsePaymentStatus(const std::string& text, PaymentStatus& value)
{
    if (text == "UNCONFIRMED") { value = PaymentStatus::Unconfirmed; return true; }
    if (text == "CONFIRMED") { value = PaymentStatus::Confirmed; return true; }
    return false;
}

bool parseTransactionStatus(const std::string& text, TransactionStatus& value)
{
    if (text == "PENDING") { value = TransactionStatus::Pending; return true; }
    if (text == "COMPLETED") { value = TransactionStatus::Completed; return true; }
    if (text == "FAILED") { value = TransactionStatus::Failed; return true; }
    return false;
}

bool parseSeatType(const std::string& text, SeatType& value)
{
    if (text == "STANDARD") { value = SeatType::Standard; return true; }
    return false;
}

bool parseTicketStatus(const std::string& text, TicketStatus& value)
{
    if (text == "ISSUED") { value = TicketStatus::Issued; return true; }
    return false;
}

std::string userRoleText(UserRole value) { return value == UserRole::Manager ? "MANAGER" : "SELLER"; }
std::string recordStatusText(RecordStatus value) { return value == RecordStatus::Active ? "ACTIVE" : "INACTIVE"; }
std::string screeningStatusText(ScreeningStatus value)
{
    switch (value)
    {
    case ScreeningStatus::Draft: return "DRAFT";
    case ScreeningStatus::Open: return "OPEN";
    case ScreeningStatus::Closed: return "CLOSED";
    case ScreeningStatus::Completed: return "COMPLETED";
    }
    return "DRAFT";
}
std::string paymentStatusText(PaymentStatus value) { return value == PaymentStatus::Confirmed ? "CONFIRMED" : "UNCONFIRMED"; }
std::string transactionStatusText(TransactionStatus value)
{
    switch (value)
    {
    case TransactionStatus::Pending: return "PENDING";
    case TransactionStatus::Completed: return "COMPLETED";
    case TransactionStatus::Failed: return "FAILED";
    }
    return "PENDING";
}
std::string seatTypeText(SeatType) { return "STANDARD"; }
std::string ticketStatusText(TicketStatus) { return "ISSUED"; }

bool splitLine(const std::string& line, DoublyLinkedList<std::string>& fields)
{
    std::string field;
    for (char character : line)
    {
        if (character == '|')
        {
            fields.addLast(field);
            field.clear();
        }
        else
        {
            field += character;
        }
    }
    fields.addLast(field);
    return true;
}

std::string fieldAt(const DoublyLinkedList<std::string>& fields, int index)
{
    const std::string* value = fields.getAt(index);
    return value == nullptr ? std::string() : *value;
}

bool hasPipeOrNewline(const std::string& value)
{
    return value.find_first_of("|\r\n") != std::string::npos || value.find('\0') != std::string::npos;
}

bool readHeader(const std::filesystem::path& path, const char* expected, std::ifstream& input, std::string& error)
{
    input.open(path);
    if (!input)
    {
        error = "Khong mo duoc file: " + path.string();
        return false;
    }

    std::string header;
    if (!std::getline(input, header))
    {
        error = "File rong hoac khong doc duoc header: " + path.string();
        return false;
    }
    if (header != expected)
    {
        error = "Header sai trong file: " + path.string();
        return false;
    }
    return true;
}

template <typename ParseRow>
bool readRows(const std::filesystem::path& path, const char* header, ParseRow parseRow, std::string& error)
{
    std::ifstream input;
    if (!readHeader(path, header, input, error)) return false;
    if (!input.is_open()) return true;

    std::string line;
    int lineNumber = 1;
    while (std::getline(input, line))
    {
        lineNumber++;
        if (line.empty()) continue;
        DoublyLinkedList<std::string> fields;
        splitLine(line, fields);
        if (!parseRow(fields, lineNumber, error)) return false;
    }
    if (!input.eof()) { error = "Loi doc file: " + path.string(); return false; }
    return true;
}

template <typename WriteRows>
bool writeAtomic(const std::filesystem::path& path, const char* header, WriteRows writeRows, std::string& error)
{
    std::error_code filesystemError;
    std::filesystem::create_directories(path.parent_path(), filesystemError);
    if (filesystemError)
    {
        error = "Khong tao duoc thu muc du lieu: " + path.parent_path().string();
        return false;
    }

    const std::filesystem::path temporary = path.string() + ".tmp";
    std::ofstream output(temporary, std::ios::trunc);
    if (!output)
    {
        error = "Khong ghi duoc file tam: " + temporary.string();
        return false;
    }
    output << header << '\n';
    writeRows(output);
    output.flush();
    if (!output)
    {
        error = "Loi ghi file: " + path.string();
        return false;
    }
    output.close();
    if (!output) { error = "Loi dong file tam: " + temporary.string(); return false; }

    std::filesystem::rename(temporary, path, filesystemError);
    if (filesystemError)
    {
        error = "Khong thay the file du lieu: " + path.string();
        std::filesystem::remove(temporary, filesystemError);
        return false;
    }
    return true;
}

bool recoverPendingWrite(const std::filesystem::path& root, std::string& error)
{
    std::error_code filesystemError;
    const auto marker = root / "write_pending.txt";
    const bool pending = std::filesystem::exists(marker, filesystemError);
    if (filesystemError) { error = "Khong kiem tra duoc write_pending.txt."; return false; }
    if (!pending) return true;
    std::ifstream input(marker);
    std::string line;
    bool existed[7]{};
    if (!std::getline(input, line) || (line != "BACKUP_READY" && line != "COMMITTED"))
    { error = "Marker khoi phuc khong hop le."; return false; }
    const bool committed = line == "COMMITTED";
    for (int index = 0; index < 7; ++index)
    {
        if (!std::getline(input, line) || (line != "0" && line != "1"))
        { error = "Marker khoi phuc thieu thong tin."; return false; }
        existed[index] = line == "1";
        const auto backup = root / (std::string(DATA_FILES[index]) + ".bak");
        if (!committed && existed[index] && (!std::filesystem::is_regular_file(backup, filesystemError) || filesystemError))
        { error = "Thieu backup; can khoi phuc thu cong: " + backup.string(); return false; }
    }
    if (std::getline(input, line)) { error = "Marker khoi phuc thua thong tin."; return false; }
    input.close();
    if (committed)
    {
        std::filesystem::remove(marker, filesystemError);
        if (filesystemError) { error = "Khong xoa duoc marker da commit."; return false; }
        for (const char* file : DATA_FILES) std::filesystem::remove(root / (std::string(file) + ".bak"), filesystemError);
        return true;
    }
    for (int index = 0; index < 7; ++index)
    {
        const auto path = root / DATA_FILES[index];
        if (existed[index])
            std::filesystem::copy_file(path.string() + ".bak", path, std::filesystem::copy_options::overwrite_existing, filesystemError);
        else std::filesystem::remove(path, filesystemError);
        if (filesystemError) { error = "Khong khoi phuc duoc file; giu nguyen backup: " + path.string(); return false; }
    }
    std::filesystem::remove(marker, filesystemError);
    if (filesystemError) { error = "Khong xoa duoc marker khoi phuc; giu backup."; return false; }
    for (const char* file : DATA_FILES) std::filesystem::remove(root / (std::string(file) + ".bak"), filesystemError);
    return true;
}

template <typename T, typename Writer>
void writeList(std::ofstream& output, const DoublyLinkedList<T>& records, Writer writer)
{
    for (int index = 0; index < records.size(); index++)
    {
        const T* record = records.getAt(index);
        if (record != nullptr) writer(output, *record);
    }
}
}

namespace cinema
{
TxtDatabase::TxtDatabase(const std::string& rootPathValue)
    : rootPath(rootPathValue), userIndex(17), filmIndex(17), auditoriumIndex(17), seatIndex(31), screeningIndex(31), transactionIndex(31), ticketIndex(31), screeningTimeIndex(compareScreeningByStart)
{
}

const DoublyLinkedList<User>& TxtDatabase::users() const { return userRecords; }
const DoublyLinkedList<Film>& TxtDatabase::films() const { return filmRecords; }
const DoublyLinkedList<Auditorium>& TxtDatabase::auditoriums() const { return auditoriumRecords; }
const DoublyLinkedList<Seat>& TxtDatabase::seats() const { return seatRecords; }
const DoublyLinkedList<Screening>& TxtDatabase::screenings() const { return screeningRecords; }
const DoublyLinkedList<SaleTransaction>& TxtDatabase::transactions() const { return transactionRecords; }
const DoublyLinkedList<Ticket>& TxtDatabase::tickets() const { return ticketRecords; }

void TxtDatabase::clearRecords()
{
    userRecords.clear();
    filmRecords.clear();
    auditoriumRecords.clear();
    seatRecords.clear();
    screeningRecords.clear();
    transactionRecords.clear();
    ticketRecords.clear();
    userIndex.clear();
    filmIndex.clear();
    auditoriumIndex.clear();
    seatIndex.clear();
    screeningIndex.clear();
    transactionIndex.clear();
    ticketIndex.clear();
    screeningTimeIndex.clear();
}

void TxtDatabase::rebuildIndexes(std::string& error)
{
    userIndex.clear();
    filmIndex.clear();
    auditoriumIndex.clear();
    seatIndex.clear();
    screeningIndex.clear();
    transactionIndex.clear();
    ticketIndex.clear();
    screeningTimeIndex.clear();

    for (int index = 0; index < userRecords.size(); index++)
    {
        const User* record = userRecords.getAt(index);
        if (record == nullptr || record->userId.empty() || !userIndex.insert("id:" + record->userId, index)) { error = "Trung hoac rong user_id."; return; }
        if (record->username.empty() || !userIndex.insert("username:" + record->username, index)) { error = "Trung username: " + record->username; return; }
    }
    for (int index = 0; index < filmRecords.size(); index++)
    {
        const Film* record = filmRecords.getAt(index);
        if (record == nullptr || record->filmId.empty() || !filmIndex.insert(record->filmId, index)) { error = "Trung hoac rong film_id."; return; }
    }
    for (int index = 0; index < auditoriumRecords.size(); index++)
    {
        const Auditorium* record = auditoriumRecords.getAt(index);
        if (record == nullptr || record->auditoriumId.empty() || !auditoriumIndex.insert(record->auditoriumId, index)) { error = "Trung hoac rong auditorium_id."; return; }
    }
    for (int index = 0; index < seatRecords.size(); index++)
    {
        const Seat* record = seatRecords.getAt(index);
        if (record == nullptr || record->seatId.empty() || !seatIndex.insert(record->seatId, index)) { error = "Trung hoac rong seat_id."; return; }
        for (int other = 0; other < index; other++)
        {
            const Seat* previous = seatRecords.getAt(other);
            if (previous != nullptr && previous->auditoriumId == record->auditoriumId && previous->seatCode == record->seatCode)
            {
                error = "Trung seat_code trong phong: " + record->seatCode;
                return;
            }
        }
    }
    for (int index = 0; index < screeningRecords.size(); index++)
    {
        const Screening* record = screeningRecords.getAt(index);
        if (record == nullptr || record->screeningId.empty() || !screeningIndex.insert(record->screeningId, index) || !screeningTimeIndex.insert(*record))
        {
            error = "Trung hoac rong screening_id/thoi diem.";
            return;
        }
    }
    for (int index = 0; index < transactionRecords.size(); index++)
    {
        const SaleTransaction* record = transactionRecords.getAt(index);
        if (record == nullptr || record->transactionId.empty() || !transactionIndex.insert(record->transactionId, index)) { error = "Trung hoac rong transaction_id."; return; }
    }
    for (int index = 0; index < ticketRecords.size(); index++)
    {
        const Ticket* record = ticketRecords.getAt(index);
        if (record == nullptr || record->ticketId.empty() || !ticketIndex.insert(record->ticketId, index)) { error = "Trung hoac rong ticket_id."; return; }
        for (int other = 0; other < index; other++)
        {
            const Ticket* previous = ticketRecords.getAt(other);
            if (previous != nullptr && previous->screeningId == record->screeningId && previous->seatId == record->seatId)
            {
                error = "Ban trung ghe trong cung suat: " + record->seatId;
                return;
            }
        }
    }
}

User* TxtDatabase::findUser(const std::string& userId) const
{
    const int* index = userIndex.search("id:" + userId);
    return index == nullptr ? nullptr : userRecords.getAt(*index);
}

User* TxtDatabase::findUserByUsername(const std::string& username) const
{
    const int* index = userIndex.search("username:" + username);
    return index == nullptr ? nullptr : userRecords.getAt(*index);
}
Film* TxtDatabase::findFilm(const std::string& filmId) const { const int* index = filmIndex.search(filmId); return index == nullptr ? nullptr : filmRecords.getAt(*index); }
Auditorium* TxtDatabase::findAuditorium(const std::string& auditoriumId) const { const int* index = auditoriumIndex.search(auditoriumId); return index == nullptr ? nullptr : auditoriumRecords.getAt(*index); }
Seat* TxtDatabase::findSeat(const std::string& seatId) const { const int* index = seatIndex.search(seatId); return index == nullptr ? nullptr : seatRecords.getAt(*index); }
Screening* TxtDatabase::findScreening(const std::string& screeningId) const { const int* index = screeningIndex.search(screeningId); return index == nullptr ? nullptr : screeningRecords.getAt(*index); }
SaleTransaction* TxtDatabase::findTransaction(const std::string& transactionId) const { const int* index = transactionIndex.search(transactionId); return index == nullptr ? nullptr : transactionRecords.getAt(*index); }
Ticket* TxtDatabase::findTicket(const std::string& ticketId) const { const int* index = ticketIndex.search(ticketId); return index == nullptr ? nullptr : ticketRecords.getAt(*index); }

bool TxtDatabase::validateReferences(std::string& error) const
{
    const auto validText = [](const std::string& text) {
        return text.find_first_not_of(" \t\r\n\f\v") != std::string::npos && !hasPipeOrNewline(text);
    };
    const auto validStatus = [](RecordStatus status) { return status == RecordStatus::Active || status == RecordStatus::Inactive; };
    for (const Node<User>* node = userRecords.getHead(); node != nullptr; node = node->next)
    {
        const auto& user = node->data;
        if (!validText(user.userId) || !validText(user.username) || !validText(user.displayName) ||
            user.passwordValue.empty() || hasPipeOrNewline(user.passwordValue) || !validStatus(user.status) ||
            (user.role != UserRole::Manager && user.role != UserRole::Seller))
        { error = "User co truong bat buoc khong hop le: " + user.userId; return false; }
    }
    for (const Node<Film>* node = filmRecords.getHead(); node != nullptr; node = node->next)
    {
        const auto& film = node->data;
        if (!validText(film.filmId) || !validText(film.title) || !validText(film.ageRating) || film.durationMinutes <= 0 || !validStatus(film.status))
        { error = "Phim khong hop le: " + film.filmId; return false; }
    }
    HashTable<int> seatCounts;
    HashTable<bool> positions;
    for (const Node<Auditorium>* node = auditoriumRecords.getHead(); node != nullptr; node = node->next)
    {
        const auto& room = node->data;
        if (!validText(room.auditoriumId) || !validText(room.name) || room.rows < 1 || room.rows > 26 || room.columns < 1 ||
            static_cast<long long>(room.rows) * room.columns != room.capacity || !validStatus(room.status))
        { error = "Phong/suc chua khong hop le: " + room.auditoriumId; return false; }
        seatCounts.insert(room.auditoriumId, 0);
    }
    for (int index = 0; index < seatRecords.size(); index++)
    {
        const Seat* seat = seatRecords.getAt(index);
        const Auditorium* room = seat == nullptr ? nullptr : findAuditorium(seat->auditoriumId);
        if (room == nullptr) { error = "Seat tham chieu phong khong ton tai."; return false; }
        if (!validText(seat->seatId) || !validText(seat->seatCode) || !validStatus(seat->status) || seat->type != SeatType::Standard || seat->rowLabel.size() != 1 ||
            seat->rowLabel[0] < 'A' || seat->rowLabel[0] >= 'A' + room->rows || seat->seatNumber < 1 ||
            seat->seatNumber > room->columns ||
            !positions.insert(seat->auditoriumId + "|" + seat->rowLabel + ":" + std::to_string(seat->seatNumber), true))
        { error = "Ghe sai hoac trung vi tri: " + seat->seatId; return false; }
        ++*seatCounts.search(seat->auditoriumId);
    }
    for (const Node<Auditorium>* node = auditoriumRecords.getHead(); node != nullptr; node = node->next)
    {
        if (*seatCounts.search(node->data.auditoriumId) != node->data.capacity)
        { error = "So ghe khong khop suc chua: " + node->data.auditoriumId; return false; }
    }
    for (int index = 0; index < screeningRecords.size(); index++)
    {
        const Screening* screening = screeningRecords.getAt(index);
        if (screening == nullptr || findFilm(screening->filmId) == nullptr || findAuditorium(screening->auditoriumId) == nullptr)
        {
            error = "Screening tham chieu phim/phong khong ton tai.";
            return false;
        }
        std::string expectedEnd, expectedAvailable;
        std::time_t availableEpoch;
        if (!validText(screening->screeningId) || screening->ticketPrice <= 0 ||
            (screening->status != ScreeningStatus::Draft && screening->status != ScreeningStatus::Open &&
             screening->status != ScreeningStatus::Closed && screening->status != ScreeningStatus::Completed) ||
            !cinema::calculateTimes(screening->startTime, findFilm(screening->filmId)->durationMinutes, expectedEnd, expectedAvailable) ||
            screening->endTime != expectedEnd || !cinema::parseTimestamp(screening->roomAvailableUntil, availableEpoch) ||
            screening->roomAvailableUntil < expectedAvailable)
        { error = "Thoi gian/gia suat chieu khong hop le: " + screening->screeningId; return false; }
        for (int other = 0; other < index; other++)
        {
            const Screening* previous = screeningRecords.getAt(other);
            if (previous != nullptr && previous->auditoriumId == screening->auditoriumId && screening->startTime < previous->roomAvailableUntil && previous->startTime < screening->roomAvailableUntil)
            {
                error = "Xung dot lich trong phong: " + screening->auditoriumId;
                return false;
            }
        }
    }
    for (int index = 0; index < transactionRecords.size(); index++)
    {
        const SaleTransaction* transaction = transactionRecords.getAt(index);
        if (transaction == nullptr || findScreening(transaction->screeningId) == nullptr || findUser(transaction->createdBy) == nullptr)
        {
            error = "Transaction tham chieu khong hop le.";
            return false;
        }
        std::time_t epoch;
        if (!validText(transaction->transactionId) || transaction->totalAmount < 0 ||
            (transaction->status != TransactionStatus::Pending && transaction->status != TransactionStatus::Completed && transaction->status != TransactionStatus::Failed) ||
            (transaction->paymentStatus != PaymentStatus::Unconfirmed && transaction->paymentStatus != PaymentStatus::Confirmed) ||
            !cinema::parseTimestamp(transaction->createdAt, epoch) ||
            transaction->createdAt >= findScreening(transaction->screeningId)->startTime ||
            ((transaction->status == TransactionStatus::Completed) != (transaction->paymentStatus == PaymentStatus::Confirmed)))
        { error = "Trang thai/thoi gian giao dich khong hop le: " + transaction->transactionId; return false; }
    }
    HashTable<long long> totals;
    HashTable<int> ticketCounts;
    for (int index = 0; index < ticketRecords.size(); index++)
    {
        const Ticket* ticket = ticketRecords.getAt(index);
        SaleTransaction* transaction = ticket == nullptr ? nullptr : findTransaction(ticket->transactionId);
        Screening* screening = ticket == nullptr ? nullptr : findScreening(ticket->screeningId);
        Seat* seat = ticket == nullptr ? nullptr : findSeat(ticket->seatId);
        if (transaction == nullptr || screening == nullptr || seat == nullptr || transaction->screeningId != ticket->screeningId || seat->auditoriumId != screening->auditoriumId)
        {
            error = "Ticket tham chieu khong hop le: " + (ticket == nullptr ? std::string() : ticket->ticketId);
            return false;
        }
        if (!validText(ticket->ticketId) || ticket->unitPrice <= 0 || ticket->status != TicketStatus::Issued || transaction->status != TransactionStatus::Completed ||
            transaction->paymentStatus != PaymentStatus::Confirmed)
        { error = "Ve phai thuoc giao dich da xac nhan: " + ticket->ticketId; return false; }
        if (!totals.contains(transaction->transactionId))
        {
            totals.insert(transaction->transactionId, 0);
            ticketCounts.insert(transaction->transactionId, 0);
        }
        long long* total = totals.search(transaction->transactionId);
        if (*total > std::numeric_limits<long long>::max() - ticket->unitPrice)
        { error = "Tong gia ve vuot gioi han."; return false; }
        *total += ticket->unitPrice;
        ++*ticketCounts.search(transaction->transactionId);
    }
    for (const Node<SaleTransaction>* node = transactionRecords.getHead(); node != nullptr; node = node->next)
    {
        if (node->data.status != TransactionStatus::Completed) continue;
        const long long* total = totals.search(node->data.transactionId);
        const int* count = ticketCounts.search(node->data.transactionId);
        if (total == nullptr || count == nullptr || *count < 1 || *total != node->data.totalAmount)
        { error = "Giao dich thieu ve hoac lech tong tien: " + node->data.transactionId; return false; }
    }
    return true;
}

bool TxtDatabase::load(std::string& error)
{
    error.clear();
    clearRecords();
    const std::filesystem::path root(rootPath);
    if (!recoverPendingWrite(root, error)) return false;

    int existingFiles = 0;
    std::string missingFile;
    for (const char* name : DATA_FILES)
    {
        std::error_code filesystemError;
        if (std::filesystem::exists(root / name, filesystemError)) ++existingFiles;
        else missingFile = name;
        if (filesystemError) { error = "Khong kiem tra duoc file: " + (root / name).string(); return false; }
    }
    if (existingFiles == 0)
    {
        for (const char* name : DATA_FILES)
        {
            std::error_code filesystemError;
            const bool backupExists = std::filesystem::exists(root / (std::string(name) + ".bak"), filesystemError);
            if (filesystemError || backupExists)
            { error = "Bo du lieu da co backup nhung mat cac file chinh; can khoi phuc thu cong."; return false; }
        }
        std::error_code filesystemError;
        const bool journalExists = std::filesystem::exists(root / "sale_journal.txt", filesystemError);
        if (filesystemError || journalExists)
        { error = "Bo du lieu da co journal nhung mat cac file chinh; can khoi phuc thu cong."; return false; }
        return saveAll(error);
    }
    if (existingFiles != 7)
    { error = "Bo du lieu thieu file: " + (root / missingFile).string() + ". Khong tu khoi tao lai; can khoi phuc backup."; return false; }

    bool ok = true;
    ok = ok && readRows(root / "users.txt", HEADER_USERS, [&](const auto& fields, int line, std::string& rowError) {
        if (fields.size() != 7) { rowError = "users.txt dong " + std::to_string(line) + ": sai so truong."; return false; }
        User value; bool canSell = false;
        if (!parseUserRole(fieldAt(fields, 4), value.role) || !isBoolean(fieldAt(fields, 5), canSell) || !parseRecordStatus(fieldAt(fields, 6), value.status)) { rowError = "users.txt dong " + std::to_string(line) + ": enum/boolean sai."; return false; }
        value.userId=fieldAt(fields,0); value.username=fieldAt(fields,1); value.passwordValue=fieldAt(fields,2); value.displayName=fieldAt(fields,3); value.canSellTicket=canSell;
        userRecords.addLast(value); return true;
    }, error);
    ok = ok && readRows(root / "films.txt", HEADER_FILMS, [&](const auto& fields, int line, std::string& rowError) {
        if (fields.size() != 5) { rowError = "films.txt dong " + std::to_string(line) + ": sai so truong."; return false; }
        Film value; if (!parseInt(fieldAt(fields,2),value.durationMinutes) || !parseRecordStatus(fieldAt(fields,4),value.status)) { rowError = "films.txt dong " + std::to_string(line) + ": so/status sai."; return false; }
        value.filmId=fieldAt(fields,0); value.title=fieldAt(fields,1); value.ageRating=fieldAt(fields,3); filmRecords.addLast(value); return true;
    }, error);
    ok = ok && readRows(root / "auditoriums.txt", HEADER_AUDITORIUMS, [&](const auto& fields, int line, std::string& rowError) {
        if (fields.size() != 6) { rowError = "auditoriums.txt dong " + std::to_string(line) + ": sai so truong."; return false; }
        Auditorium value; if (!parseInt(fieldAt(fields,2),value.rows) || !parseInt(fieldAt(fields,3),value.columns) || !parseInt(fieldAt(fields,4),value.capacity) || !parseRecordStatus(fieldAt(fields,5),value.status)) { rowError = "auditoriums.txt dong " + std::to_string(line) + ": so/status sai."; return false; }
        value.auditoriumId=fieldAt(fields,0); value.name=fieldAt(fields,1); auditoriumRecords.addLast(value); return true;
    }, error);
    ok = ok && readRows(root / "seats.txt", HEADER_SEATS, [&](const auto& fields, int line, std::string& rowError) {
        if (fields.size() != 7) { rowError = "seats.txt dong " + std::to_string(line) + ": sai so truong."; return false; }
        Seat value; if (!parseInt(fieldAt(fields,4),value.seatNumber) || !parseSeatType(fieldAt(fields,5),value.type) || !parseRecordStatus(fieldAt(fields,6),value.status)) { rowError = "seats.txt dong " + std::to_string(line) + ": so/status sai."; return false; }
        value.seatId=fieldAt(fields,0); value.auditoriumId=fieldAt(fields,1); value.seatCode=fieldAt(fields,2); value.rowLabel=fieldAt(fields,3); seatRecords.addLast(value); return true;
    }, error);
    ok = ok && readRows(root / "screenings.txt", HEADER_SCREENINGS, [&](const auto& fields, int line, std::string& rowError) {
        if (fields.size() != 8) { rowError = "screenings.txt dong " + std::to_string(line) + ": sai so truong."; return false; }
        Screening value; if (!parseMoney(fieldAt(fields,6),value.ticketPrice) || !parseScreeningStatus(fieldAt(fields,7),value.status)) { rowError = "screenings.txt dong " + std::to_string(line) + ": gia/status sai."; return false; }
        value.screeningId=fieldAt(fields,0); value.filmId=fieldAt(fields,1); value.auditoriumId=fieldAt(fields,2); value.startTime=fieldAt(fields,3); value.endTime=fieldAt(fields,4); value.roomAvailableUntil=fieldAt(fields,5); screeningRecords.addLast(value); return true;
    }, error);
    ok = ok && readRows(root / "transactions.txt", HEADER_TRANSACTIONS, [&](const auto& fields, int line, std::string& rowError) {
        if (fields.size() != 7) { rowError = "transactions.txt dong " + std::to_string(line) + ": sai so truong."; return false; }
        SaleTransaction value; if (!parseMoney(fieldAt(fields,4),value.totalAmount) || !parsePaymentStatus(fieldAt(fields,5),value.paymentStatus) || !parseTransactionStatus(fieldAt(fields,6),value.status)) { rowError = "transactions.txt dong " + std::to_string(line) + ": gia/status sai."; return false; }
        value.transactionId=fieldAt(fields,0); value.screeningId=fieldAt(fields,1); value.createdAt=fieldAt(fields,2); value.createdBy=fieldAt(fields,3); transactionRecords.addLast(value); return true;
    }, error);
    ok = ok && readRows(root / "tickets.txt", HEADER_TICKETS, [&](const auto& fields, int line, std::string& rowError) {
        if (fields.size() != 6) { rowError = "tickets.txt dong " + std::to_string(line) + ": sai so truong."; return false; }
        Ticket value; if (!parseMoney(fieldAt(fields,4),value.unitPrice) || !parseTicketStatus(fieldAt(fields,5),value.status)) { rowError = "tickets.txt dong " + std::to_string(line) + ": gia/status sai."; return false; }
        value.ticketId=fieldAt(fields,0); value.transactionId=fieldAt(fields,1); value.screeningId=fieldAt(fields,2); value.seatId=fieldAt(fields,3); ticketRecords.addLast(value); return true;
    }, error);
    if (!ok) { clearRecords(); return false; }
    rebuildIndexes(error);
    if (!error.empty()) { clearRecords(); return false; }
    if (!validateReferences(error)) { clearRecords(); return false; }
    return true;
}

bool TxtDatabase::saveAll(std::string& error) const
{
    error.clear();
    if (!validateReferences(error)) return false;
    const std::filesystem::path root(rootPath);
    if (!recoverPendingWrite(root, error)) return false;
    std::filesystem::path backups[7];
    bool existed[7]{};
    std::error_code filesystemError;
    std::filesystem::create_directories(root, filesystemError);
    if (filesystemError) { error = "Khong tao duoc thu muc du lieu."; return false; }
    for (int index = 0; index < 7; ++index)
    {
        const std::filesystem::path path = root / DATA_FILES[index];
        backups[index] = path.string() + ".bak";
        existed[index] = std::filesystem::exists(path, filesystemError);
        if (filesystemError) { error = "Khong kiem tra duoc file du lieu."; return false; }
        if (existed[index] && (!std::filesystem::is_regular_file(path, filesystemError) || filesystemError ||
                               !std::filesystem::copy_file(path, backups[index], std::filesystem::copy_options::overwrite_existing, filesystemError)))
        { error = "Khong tao duoc backup du lieu."; return false; }
    }

    const auto marker = root / "write_pending.txt";
    if (!writeAtomic(marker, "BACKUP_READY", [&](std::ofstream& output) {
        for (bool value : existed) output << (value ? "1\n" : "0\n");
    }, error)) return false;

    bool ok = writeAtomic(root / "users.txt", HEADER_USERS, [&](std::ofstream& output) { writeList(output, userRecords, [](std::ofstream& out, const User& v) { out << v.userId << '|' << v.username << '|' << v.passwordValue << '|' << v.displayName << '|' << userRoleText(v.role) << '|' << (v.canSellTicket ? 1 : 0) << '|' << recordStatusText(v.status) << '\n'; }); }, error);
    ok = ok && writeAtomic(root / "films.txt", HEADER_FILMS, [&](std::ofstream& output) { writeList(output, filmRecords, [](std::ofstream& out, const Film& v) { out << v.filmId << '|' << v.title << '|' << v.durationMinutes << '|' << v.ageRating << '|' << recordStatusText(v.status) << '\n'; }); }, error);
    ok = ok && writeAtomic(root / "auditoriums.txt", HEADER_AUDITORIUMS, [&](std::ofstream& output) { writeList(output, auditoriumRecords, [](std::ofstream& out, const Auditorium& v) { out << v.auditoriumId << '|' << v.name << '|' << v.rows << '|' << v.columns << '|' << v.capacity << '|' << recordStatusText(v.status) << '\n'; }); }, error);
    ok = ok && writeAtomic(root / "seats.txt", HEADER_SEATS, [&](std::ofstream& output) { writeList(output, seatRecords, [](std::ofstream& out, const Seat& v) { out << v.seatId << '|' << v.auditoriumId << '|' << v.seatCode << '|' << v.rowLabel << '|' << v.seatNumber << '|' << seatTypeText(v.type) << '|' << recordStatusText(v.status) << '\n'; }); }, error);
    ok = ok && writeAtomic(root / "screenings.txt", HEADER_SCREENINGS, [&](std::ofstream& output) { writeList(output, screeningRecords, [](std::ofstream& out, const Screening& v) { out << v.screeningId << '|' << v.filmId << '|' << v.auditoriumId << '|' << v.startTime << '|' << v.endTime << '|' << v.roomAvailableUntil << '|' << v.ticketPrice << '|' << screeningStatusText(v.status) << '\n'; }); }, error);
    ok = ok && writeAtomic(root / "transactions.txt", HEADER_TRANSACTIONS, [&](std::ofstream& output) { writeList(output, transactionRecords, [](std::ofstream& out, const SaleTransaction& v) { out << v.transactionId << '|' << v.screeningId << '|' << v.createdAt << '|' << v.createdBy << '|' << v.totalAmount << '|' << paymentStatusText(v.paymentStatus) << '|' << transactionStatusText(v.status) << '\n'; }); }, error);
    ok = ok && writeAtomic(root / "tickets.txt", HEADER_TICKETS, [&](std::ofstream& output) { writeList(output, ticketRecords, [](std::ofstream& out, const Ticket& v) { out << v.ticketId << '|' << v.transactionId << '|' << v.screeningId << '|' << v.seatId << '|' << v.unitPrice << '|' << ticketStatusText(v.status) << '\n'; }); }, error);
    if (!ok)
    {
        std::string recoveryError;
        if (!recoverPendingWrite(root, recoveryError)) error += " " + recoveryError;
        return false;
    }
    if (!writeAtomic(marker, "COMMITTED", [&](std::ofstream& output) {
        for (bool value : existed) output << (value ? "1\n" : "0\n");
    }, error))
    {
        std::string recoveryError;
        if (!recoverPendingWrite(root, recoveryError)) error += " " + recoveryError;
        return false;
    }
    std::filesystem::remove(marker, filesystemError);
    if (filesystemError) return true;
    for (const std::filesystem::path& backup : backups) std::filesystem::remove(backup, filesystemError);
    return true;
}

bool TxtDatabase::persistMutation(const std::function<bool()>& mutation, std::string& error)
{
    error.clear();
    // ponytail: copy all seven tables for rollback; use table-level undo if datasets grow.
    const auto oldUsers = userRecords;
    const auto oldFilms = filmRecords;
    const auto oldAuditoriums = auditoriumRecords;
    const auto oldSeats = seatRecords;
    const auto oldScreenings = screeningRecords;
    const auto oldTransactions = transactionRecords;
    const auto oldTickets = ticketRecords;
    bool ok = mutation();
    if (ok) { rebuildIndexes(error); ok = error.empty() && validateReferences(error); }
    if (ok) ok = saveAll(error);
    if (ok) return true;
    userRecords = oldUsers;
    filmRecords = oldFilms;
    auditoriumRecords = oldAuditoriums;
    seatRecords = oldSeats;
    screeningRecords = oldScreenings;
    transactionRecords = oldTransactions;
    ticketRecords = oldTickets;
    std::string restoreError;
    rebuildIndexes(restoreError);
    return false;
}

bool TxtDatabase::saveUser(const User& user, std::string& error)
{
    return persistMutation([&] {
        if (user.userId.empty() || user.username.empty() || user.passwordValue.empty() || user.displayName.empty() ||
            hasPipeOrNewline(user.userId) || hasPipeOrNewline(user.username) || hasPipeOrNewline(user.passwordValue) || hasPipeOrNewline(user.displayName) ||
            (user.role != UserRole::Manager && user.role != UserRole::Seller) ||
            (user.status != RecordStatus::Active && user.status != RecordStatus::Inactive))
        { error = "Tai khoan khong hop le."; return false; }
        User* existing = findUser(user.userId);
        const User* usernameOwner = findUserByUsername(user.username);
        if (usernameOwner != nullptr && usernameOwner->userId != user.userId)
        { error = "Username da ton tai."; return false; }
        if (existing != nullptr) *existing = user;
        else userRecords.addLast(user);
        return true;
    }, error);
}

bool TxtDatabase::saveFilm(const Film& film, std::string& error)
{
    return persistMutation([&] {
        if (film.filmId.empty() || film.title.empty() || film.ageRating.empty() || film.durationMinutes <= 0 ||
            hasPipeOrNewline(film.filmId) || hasPipeOrNewline(film.title) || hasPipeOrNewline(film.ageRating) ||
            (film.status != RecordStatus::Active && film.status != RecordStatus::Inactive))
        { error = "Phim khong hop le."; return false; }
        Film* existing = findFilm(film.filmId);
        if (existing != nullptr) *existing = film;
        else filmRecords.addLast(film);
        return true;
    }, error);
}

bool TxtDatabase::saveAuditorium(const Auditorium& auditorium, const DoublyLinkedList<Seat>& seats, std::string& error)
{
    return persistMutation([&] {
        if (auditorium.auditoriumId.empty() || auditorium.name.empty() || auditorium.rows < 1 || auditorium.rows > 26 || auditorium.columns < 1 ||
            static_cast<long long>(auditorium.rows) * auditorium.columns != auditorium.capacity || seats.size() != auditorium.capacity ||
            hasPipeOrNewline(auditorium.auditoriumId) || hasPipeOrNewline(auditorium.name) ||
            (auditorium.status != RecordStatus::Active && auditorium.status != RecordStatus::Inactive))
        { error = "Phong/so do ghe khong hop le."; return false; }
        HashTable<int> positions;
        for (const Node<Seat>* node = seats.getHead(); node != nullptr; node = node->next)
        {
            const Seat& seat = node->data;
            if (seat.seatId.empty() || seat.seatCode.empty() || seat.auditoriumId != auditorium.auditoriumId || seat.rowLabel.size() != 1 ||
                seat.rowLabel[0] < 'A' || seat.rowLabel[0] >= 'A' + auditorium.rows || seat.seatNumber < 1 || seat.seatNumber > auditorium.columns ||
                hasPipeOrNewline(seat.seatId) || hasPipeOrNewline(seat.seatCode) || seat.type != SeatType::Standard ||
                (seat.status != RecordStatus::Active && seat.status != RecordStatus::Inactive) ||
                !positions.insert(seat.rowLabel + ":" + std::to_string(seat.seatNumber), 1))
            { error = "Ghe khong hop le hoac trung vi tri."; return false; }
        }
        for (int index = seatRecords.size() - 1; index >= 0; --index)
            if (seatRecords.getAt(index)->auditoriumId == auditorium.auditoriumId) seatRecords.removeAt(index);
        Auditorium* existing = findAuditorium(auditorium.auditoriumId);
        if (existing != nullptr) *existing = auditorium;
        else auditoriumRecords.addLast(auditorium);
        for (const Node<Seat>* node = seats.getHead(); node != nullptr; node = node->next) seatRecords.addLast(node->data);
        return true;
    }, error);
}

bool TxtDatabase::saveScreening(const Screening& screening, std::string& error)
{
    return persistMutation([&] {
        if (screening.screeningId.empty() || hasPipeOrNewline(screening.screeningId) || screening.ticketPrice <= 0 ||
            screening.startTime.empty() || screening.startTime >= screening.endTime || screening.endTime >= screening.roomAvailableUntil ||
            findFilm(screening.filmId) == nullptr || findAuditorium(screening.auditoriumId) == nullptr ||
            (screening.status != ScreeningStatus::Draft && screening.status != ScreeningStatus::Open &&
             screening.status != ScreeningStatus::Closed && screening.status != ScreeningStatus::Completed))
        { error = "Suat chieu khong hop le."; return false; }
        Screening* existing = findScreening(screening.screeningId);
        if (existing != nullptr && (existing->filmId != screening.filmId || existing->auditoriumId != screening.auditoriumId ||
            existing->startTime != screening.startTime || existing->endTime != screening.endTime ||
            existing->roomAvailableUntil != screening.roomAvailableUntil || existing->ticketPrice != screening.ticketPrice))
        {
            for (const Node<Ticket>* node = ticketRecords.getHead(); node != nullptr; node = node->next)
                if (node->data.screeningId == screening.screeningId)
                { error = "Khong sua thong tin suat chieu da co ve."; return false; }
        }
        if (existing != nullptr) *existing = screening;
        else screeningRecords.addLast(screening);
        return true;
    }, error);
}

bool TxtDatabase::addUser(const User& user, std::string& error)
{
    error.clear();
    if (user.userId.empty() || user.username.empty() || hasPipeOrNewline(user.userId) || hasPipeOrNewline(user.username) || findUser(user.userId) != nullptr || findUserByUsername(user.username) != nullptr) { error = "user_id/username rong hoac trung."; return false; }
    userRecords.addLast(user); return rebuildIndexes(error), error.empty();
}

bool TxtDatabase::addFilm(const Film& film, std::string& error)
{
    error.clear();
    if (film.filmId.empty() || film.title.empty() || film.durationMinutes <= 0 || hasPipeOrNewline(film.title) || findFilm(film.filmId) != nullptr) { error = "Phim khong hop le hoac trung ma."; return false; }
    filmRecords.addLast(film); return rebuildIndexes(error), error.empty();
}

bool TxtDatabase::addAuditorium(const Auditorium& auditorium, std::string& error)
{
    error.clear();
    if (auditorium.auditoriumId.empty() || auditorium.name.empty() || auditorium.rows <= 0 || auditorium.columns <= 0 || auditorium.capacity != auditorium.rows * auditorium.columns || findAuditorium(auditorium.auditoriumId) != nullptr) { error = "Phong khong hop le hoac trung ma."; return false; }
    auditoriumRecords.addLast(auditorium); return rebuildIndexes(error), error.empty();
}

bool TxtDatabase::addSeat(const Seat& seat, std::string& error)
{
    error.clear();
    if (seat.seatId.empty() || seat.seatCode.empty() || seat.seatNumber <= 0 || findSeat(seat.seatId) != nullptr || findAuditorium(seat.auditoriumId) == nullptr) { error = "Ghe khong hop le, phong khong ton tai hoac trung ma."; return false; }
    for (int index = 0; index < seatRecords.size(); index++) { const Seat* other = seatRecords.getAt(index); if (other != nullptr && other->auditoriumId == seat.auditoriumId && other->seatCode == seat.seatCode) { error = "Trung ma ghe trong phong."; return false; } }
    seatRecords.addLast(seat); return rebuildIndexes(error), error.empty();
}

bool TxtDatabase::addScreening(const Screening& screening, std::string& error)
{
    error.clear();
    if (screening.screeningId.empty() || screening.startTime.empty() || screening.roomAvailableUntil.empty() || screening.ticketPrice <= 0 || findScreening(screening.screeningId) != nullptr || findFilm(screening.filmId) == nullptr || findAuditorium(screening.auditoriumId) == nullptr) { error = "Suat chieu khong hop le hoac tham chieu sai."; return false; }
    for (int index = 0; index < screeningRecords.size(); index++) { const Screening* other = screeningRecords.getAt(index); if (other != nullptr && other->auditoriumId == screening.auditoriumId && screening.startTime < other->roomAvailableUntil && other->startTime < screening.roomAvailableUntil) { error = "Suat chieu bi chong lich."; return false; } }
    screeningRecords.addLast(screening); return rebuildIndexes(error), error.empty();
}

bool TxtDatabase::appendJournal(const std::string& state, const std::string& transactionId, std::string& error) const
{
    std::error_code filesystemError;
    std::filesystem::create_directories(rootPath, filesystemError);
    if (filesystemError) { error = "Khong tao duoc thu muc journal."; return false; }
    std::ofstream output(std::filesystem::path(rootPath) / "sale_journal.txt", std::ios::app);
    if (!output) { error = "Khong ghi duoc sale_journal.txt."; return false; }
    output << state << '|' << transactionId << '\n';
    return static_cast<bool>(output);
}

bool TxtDatabase::commitSale(const SaleTransaction& transaction, const DoublyLinkedList<Ticket>& saleTickets, std::string& error)
{
    error.clear();
    Screening* screening = findScreening(transaction.screeningId);
    if (transaction.transactionId.empty() || transaction.totalAmount <= 0 || transaction.status != TransactionStatus::Completed || transaction.paymentStatus != PaymentStatus::Confirmed || findTransaction(transaction.transactionId) != nullptr || findUser(transaction.createdBy) == nullptr || screening == nullptr || screening->status != ScreeningStatus::Open || screening->ticketPrice <= 0 || saleTickets.isEmpty() ||
        hasPipeOrNewline(transaction.transactionId) || hasPipeOrNewline(transaction.screeningId) || hasPipeOrNewline(transaction.createdAt) || hasPipeOrNewline(transaction.createdBy)) { error = "Giao dich khong hop le hoac chua san sang."; return false; }

    long long total = 0;
    for (int index = 0; index < saleTickets.size(); index++)
    {
        const Ticket* ticket = saleTickets.getAt(index);
        if (ticket == nullptr || ticket->ticketId.empty() || ticket->screeningId != transaction.screeningId || ticket->transactionId != transaction.transactionId || ticket->unitPrice != screening->ticketPrice || ticket->status != TicketStatus::Issued || findTicket(ticket->ticketId) != nullptr || findSeat(ticket->seatId) == nullptr || hasPipeOrNewline(ticket->ticketId) || hasPipeOrNewline(ticket->seatId)) { error = "Ve trong giao dich khong hop le."; return false; }
        Seat* seat = findSeat(ticket->seatId);
        if (seat->auditoriumId != screening->auditoriumId) { error = "Ghe khong thuoc phong cua suat chieu."; return false; }
        for (int other = 0; other < index; other++) { const Ticket* previous = saleTickets.getAt(other); if (previous != nullptr && previous->seatId == ticket->seatId) { error = "Trung ghe trong giao dich."; return false; } }
        for (int existing = 0; existing < ticketRecords.size(); existing++) { const Ticket* previous = ticketRecords.getAt(existing); if (previous != nullptr && previous->screeningId == ticket->screeningId && previous->seatId == ticket->seatId) { error = "Ghe da duoc ban trong suat chieu."; return false; } }
        if (total > std::numeric_limits<long long>::max() - ticket->unitPrice) { error = "Tong tien vuot gioi han."; return false; }
        total += ticket->unitPrice;
    }
    if (total != transaction.totalAmount) { error = "Tong tien giao dich khong khop tong gia ve."; return false; }
    if (!appendJournal("PREPARED", transaction.transactionId, error)) return false;
    if (!persistMutation([&] {
        transactionRecords.addLast(transaction);
        for (const Node<Ticket>* node = saleTickets.getHead(); node != nullptr; node = node->next) ticketRecords.addLast(node->data);
        return true;
    }, error)) return false;
    std::string journalError;
    appendJournal("COMMITTED", transaction.transactionId, journalError);
    return true;
}

Matrix2D TxtDatabase::seatMapForScreening(const std::string& screeningId) const
{
    const Screening* screening = findScreening(screeningId);
    const Auditorium* auditorium = screening == nullptr ? nullptr : findAuditorium(screening->auditoriumId);
    if (screening == nullptr || auditorium == nullptr) return Matrix2D();
    Matrix2D map(auditorium->rows, auditorium->columns, '-');
    for (int index = 0; index < seatRecords.size(); index++)
    {
        const Seat* seat = seatRecords.getAt(index);
        if (seat == nullptr || seat->auditoriumId != auditorium->auditoriumId || seat->status != RecordStatus::Active || seat->rowLabel.empty()) continue;
        const int row = seat->rowLabel[0] - 'A';
        map.set(row, seat->seatNumber - 1, 'O');
    }
    for (int index = 0; index < ticketRecords.size(); index++)
    {
        const Ticket* ticket = ticketRecords.getAt(index);
        if (ticket == nullptr || ticket->screeningId != screeningId) continue;
        const SaleTransaction* transaction = findTransaction(ticket->transactionId);
        if (transaction == nullptr || transaction->status != TransactionStatus::Completed || transaction->paymentStatus != PaymentStatus::Confirmed) continue;
        const Seat* seat = findSeat(ticket->seatId);
        if (seat != nullptr && !seat->rowLabel.empty()) map.set(seat->rowLabel[0] - 'A', seat->seatNumber - 1, 'X');
    }
    return map;
}
}
