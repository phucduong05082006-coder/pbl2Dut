#include <cassert>
#include <string>

#include "entity/Film.h"
#include "entity/Screening.h"
#include "entity/Seat.h"
#include "entity/User.h"

#include "DataStructures/BST.h"
#include "DataStructures/DoublyLinkedList.h"
#include "DataStructures/HashTable.h"
#include "DataStructures/Matrix2D.h"

int main()
{
    using namespace cinema;

    Film film{"F001", "Chuyen tau mua thu", 98, "P", RecordStatus::Active};
    Film secondFilm{"F002", "Phia sau anh den", 112, "T13", RecordStatus::Active};

    DoublyLinkedList<Film> films;
    films.addLast(film);
    films.addLast(secondFilm);
    assert(films.size() == 2);
    assert(films.find(film) != nullptr);

    HashTable<int> filmIndex(7);
    assert(filmIndex.insert(film.filmId, 0));
    assert(filmIndex.insert(secondFilm.filmId, 1));
    assert(*filmIndex.search("F002") == 1);
    assert(!filmIndex.insert("F002", 2));

    Screening morning{"SC001", "F001", "R001", "2026-10-11 11:30:00", "2026-10-11 13:08:00", "2026-10-11 13:23:00", 80000, ScreeningStatus::Open};
    Screening afternoon{"SC002", "F001", "R001", "2026-10-11 15:00:00", "2026-10-11 16:38:00", "2026-10-11 16:53:00", 80000, ScreeningStatus::Open};

    BST<Screening> schedule(compareScreeningByStart);
    assert(schedule.insert(afternoon));
    assert(schedule.insert(morning));
    assert(schedule.size() == 2);
    assert(schedule.search(morning) != nullptr);

    Matrix2D seatMap(2, 3, 'O');
    assert(seatMap.count('O') == 6);
    assert(seatMap.set(0, 1, 'X'));
    assert(seatMap.get(0, 1) == 'X');
    assert(seatMap.count('X') == 1);

    User seller{"U001", "seller01", "demo", "Nhan vien", UserRole::Seller, true, RecordStatus::Active};
    assert(seller.canSellTicket);

    return 0;
}
