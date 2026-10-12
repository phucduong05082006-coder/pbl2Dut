#ifndef CINEMA_PRESENTATION_ACCOUNT_PAGE_H
#define CINEMA_PRESENTATION_ACCOUNT_PAGE_H

#include <QWidget>

#include "controller/AuthController.h"

class QLabel;
class QPushButton;
class QTableWidget;

class AccountPage final : public QWidget
{
public:
    explicit AccountPage(cinema::AuthController& auth, QWidget* parent = nullptr);
    void refreshPermissions();
    void reload();

private:
    cinema::AuthController& auth;
    QTableWidget* table;
    QLabel* errorLabel;
    QLabel* summary;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* permissionButton;
    QPushButton* statusButton;
    bool canManage = false;

    void updateActions();
    bool selectedUser(cinema::UserSummary& user);
    void showForm(bool creating);
    void toggle(bool permission);
};

#endif
