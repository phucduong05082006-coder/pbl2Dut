#ifndef CINEMA_PRESENTATION_AUDITORIUM_PAGE_H
#define CINEMA_PRESENTATION_AUDITORIUM_PAGE_H

#include <QWidget>

#include "controller/AuditoriumController.h"
#include "controller/AuthController.h"

class QComboBox;
class QLabel;
class QTableWidget;
class QPushButton;

class AuditoriumPage final : public QWidget
{
public:
    AuditoriumPage(cinema::AuditoriumController& auditoriums, cinema::AuthController& auth,
                   QWidget* parent = nullptr);
    void reload();
    void refreshPermissions();

private:
    cinema::AuditoriumController& auditoriums;
    cinema::AuthController& auth;
    QComboBox* roomChoice;
    QComboBox* statusFilter;
    QLabel* summary;
    QLabel* errorLabel;
    QTableWidget* seatTable;
    QPushButton* addButton;
    QPushButton* editButton;

    void showRoom();
    void showForm(bool creating);
};

#endif
