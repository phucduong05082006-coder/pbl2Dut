#ifndef CINEMA_PRESENTATION_FILM_PAGE_H
#define CINEMA_PRESENTATION_FILM_PAGE_H

#include <QWidget>

#include "controller/AuthController.h"
#include "controller/FilmController.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

class FilmPage final : public QWidget
{
public:
    FilmPage(cinema::FilmController& films, cinema::AuthController& auth, QWidget* parent = nullptr);
    void refreshPermissions();

private:
    cinema::FilmController& films;
    cinema::AuthController& auth;
    QLineEdit* searchInput;
    QComboBox* statusFilter;
    QTableWidget* table;
    QLabel* summary;
    QLabel* errorLabel;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* detailsButton;
    QPushButton* deactivateButton;
    bool canManage = false;

    void reload();
    void updateActions();
    bool selectedFilm(cinema::Film& film);
    void showForm(bool creating, bool readOnly = false);
    void deactivate();
};

#endif
