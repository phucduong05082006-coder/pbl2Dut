#ifndef CINEMA_PRESENTATION_OPERATIONS_PAGES_H
#define CINEMA_PRESENTATION_OPERATIONS_PAGES_H

#include <QWidget>

#include "controller/OperationsController.h"
#include "controller/AuthController.h"
#include "controller/FilmController.h"
#include "controller/AuditoriumController.h"

class QCheckBox;
class QComboBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTextEdit;

class ScreeningPage final : public QWidget
{
public:
    ScreeningPage(cinema::OperationsController& operations, cinema::FilmController& films,
        cinema::AuditoriumController& rooms, cinema::AuthController& auth, QWidget* parent = nullptr);
    void reload();
    void refreshPermissions();

private:
    cinema::OperationsController& operations;
    cinema::FilmController& films;
    cinema::AuditoriumController& rooms;
    cinema::AuthController& auth;
    QComboBox* filmFilter;
    QComboBox* roomFilter;
    QComboBox* statusFilter;
    QLineEdit* dateFilter;
    QTableWidget* table;
    QLabel* errorLabel;
    QPushButton* addButton;
    QPushButton* openButton;
    QPushButton* closeButton;
    QPushButton* completeButton;
    void create();
    void changeStatus(cinema::ScreeningStatus status);
};

class SalePage final : public QWidget
{
public:
    SalePage(cinema::OperationsController& operations, cinema::FilmController& films,
        cinema::AuditoriumController& rooms, cinema::AuthController& auth, QWidget* parent = nullptr);
    void reload();
    void refreshPermissions();

private:
    cinema::OperationsController& operations;
    cinema::FilmController& films;
    cinema::AuditoriumController& rooms;
    cinema::AuthController& auth;
    QComboBox* filmFilter;
    QComboBox* roomFilter;
    QLineEdit* dateFilter;
    QComboBox* showChoice;
    QTableWidget* seats;
    QLabel* summary;
    QLabel* errorLabel;
    QPushButton* previewButton;
    QPushButton* cancelButton;
    void showSeats();
    void checkout();
};

class LookupPage final : public QWidget
{
public:
    explicit LookupPage(cinema::OperationsController& operations, QWidget* parent = nullptr);
    void clear();

private:
    cinema::OperationsController& operations;
    QLineEdit* idInput;
    QComboBox* type;
    QLabel* errorLabel;
    QTextEdit* result;
};

class ReportPage final : public QWidget
{
public:
    explicit ReportPage(cinema::OperationsController& operations, QWidget* parent = nullptr);
    void reload();
    void clear();

private:
    cinema::OperationsController& operations;
    QDateEdit* from;
    QDateEdit* to;
    QComboBox* filmFilter;
    QComboBox* screeningFilter;
    QLabel* summary;
    QLabel* errorLabel;
    QTableWidget* table;
    QTableWidget* occupancy;
    void run();
};

#endif
