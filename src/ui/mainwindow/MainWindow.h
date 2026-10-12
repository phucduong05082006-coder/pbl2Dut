#ifndef CINEMA_PRESENTATION_MAIN_WINDOW_H
#define CINEMA_PRESENTATION_MAIN_WINDOW_H

#include <QMainWindow>

#include "controller/AuthController.h"
#include "controller/FilmController.h"
#include "controller/AuditoriumController.h"
#include "controller/OperationsController.h"

class QLabel;
class QPushButton;
class QStackedWidget;
class FilmPage;
class AccountPage;
class AuditoriumPage;
class ScreeningPage;
class SalePage;
class LookupPage;
class ReportPage;

class MainWindow final : public QMainWindow
{
public:
    MainWindow(cinema::AuthController& auth, cinema::FilmController& films,
               cinema::AuditoriumController& auditoriums, cinema::OperationsController& operations,
               const QString& dataPath = {}, QWidget* parent = nullptr);

private:
    cinema::AuthController& auth;
    FilmPage* filmPage;
    AccountPage* accountPage;
    AuditoriumPage* auditoriumPage;
    ScreeningPage* screeningPage;
    SalePage* salePage;
    LookupPage* lookupPage;
    ReportPage* reportPage;
    QWidget* overviewPage;
    QPushButton* filmsNavigation;
    QPushButton* accountsNavigation;
    QPushButton* auditoriumsNavigation;
    QPushButton* overviewNavigation;
    QPushButton* screeningsNavigation;
    QPushButton* salesNavigation;
    QPushButton* lookupNavigation;
    QPushButton* reportsNavigation;
    QStackedWidget* pages;
    QLabel* pageTitle;
    QLabel* pageDescription;
    QLabel* identity;
    QLabel* permissions;

    void refreshSession();
};

#endif
