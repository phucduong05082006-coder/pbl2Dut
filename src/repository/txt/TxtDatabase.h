#ifndef CINEMA_REPOSITORY_TXT_DATABASE_H
#define CINEMA_REPOSITORY_TXT_DATABASE_H

#include <functional>
#include <string>

#include "DataStructures/BST.h"
#include "DataStructures/DoublyLinkedList.h"
#include "DataStructures/HashTable.h"
#include "DataStructures/Matrix2D.h"

#include "../../entity/Auditorium.h"
#include "../../entity/Film.h"
#include "../../entity/SaleTransaction.h"
#include "../../entity/Screening.h"
#include "../../entity/Seat.h"
#include "../../entity/Ticket.h"
#include "../../entity/User.h"

namespace cinema
{
class TxtDatabase final
{
public:
    explicit TxtDatabase(const std::string& rootPath);

    bool load(std::string& error);
    bool saveAll(std::string& error) const;

    const DoublyLinkedList<User>& users() const;
    const DoublyLinkedList<Film>& films() const;
    const DoublyLinkedList<Auditorium>& auditoriums() const;
    const DoublyLinkedList<Seat>& seats() const;
    const DoublyLinkedList<Screening>& screenings() const;
    const DoublyLinkedList<SaleTransaction>& transactions() const;
    const DoublyLinkedList<Ticket>& tickets() const;

    User* findUser(const std::string& userId) const;
    User* findUserByUsername(const std::string& username) const;
    Film* findFilm(const std::string& filmId) const;
    Auditorium* findAuditorium(const std::string& auditoriumId) const;
    Seat* findSeat(const std::string& seatId) const;
    Screening* findScreening(const std::string& screeningId) const;
    SaleTransaction* findTransaction(const std::string& transactionId) const;
    Ticket* findTicket(const std::string& ticketId) const;

    bool addUser(const User& user, std::string& error);
    bool addFilm(const Film& film, std::string& error);
    bool addAuditorium(const Auditorium& auditorium, std::string& error);
    bool addSeat(const Seat& seat, std::string& error);
    bool addScreening(const Screening& screening, std::string& error);

    bool saveUser(const User& user, std::string& error);
    bool saveFilm(const Film& film, std::string& error);
    bool saveAuditorium(const Auditorium& auditorium, const DoublyLinkedList<Seat>& seats, std::string& error);
    bool saveScreening(const Screening& screening, std::string& error);

    bool commitSale(const SaleTransaction& transaction,
                    const DoublyLinkedList<Ticket>& saleTickets,
                    std::string& error);

    Matrix2D seatMapForScreening(const std::string& screeningId) const;

private:
    std::string rootPath;

    DoublyLinkedList<User> userRecords;
    DoublyLinkedList<Film> filmRecords;
    DoublyLinkedList<Auditorium> auditoriumRecords;
    DoublyLinkedList<Seat> seatRecords;
    DoublyLinkedList<Screening> screeningRecords;
    DoublyLinkedList<SaleTransaction> transactionRecords;
    DoublyLinkedList<Ticket> ticketRecords;

    HashTable<int> userIndex;
    HashTable<int> filmIndex;
    HashTable<int> auditoriumIndex;
    HashTable<int> seatIndex;
    HashTable<int> screeningIndex;
    HashTable<int> transactionIndex;
    HashTable<int> ticketIndex;
    BST<Screening> screeningTimeIndex;

    void clearRecords();
    void rebuildIndexes(std::string& error);
    bool validateReferences(std::string& error) const;
    bool appendJournal(const std::string& state, const std::string& transactionId, std::string& error) const;
    bool persistMutation(const std::function<bool()>& mutation, std::string& error);
};
}

#endif
