#ifndef CINEMA_PRESENTATION_LOGIN_DIALOG_H
#define CINEMA_PRESENTATION_LOGIN_DIALOG_H

#include <QDialog>

#include "controller/AuthController.h"

class QLabel;
class QLineEdit;
class QPushButton;

class LoginDialog final : public QDialog
{
public:
    explicit LoginDialog(cinema::AuthController& auth, QWidget* parent = nullptr);

private:
    cinema::AuthController& auth;
    QLineEdit* username;
    QLineEdit* password;
    QLabel* errorLabel;
    QPushButton* submitButton;

    void submit();
    void reject() override;
};

#endif
