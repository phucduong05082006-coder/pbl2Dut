#ifndef CINEMA_PRESENTATION_ADMIN_SETUP_H
#define CINEMA_PRESENTATION_ADMIN_SETUP_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

#include "controller/AuthController.h"

inline bool runAdminSetup(cinema::AuthController& auth)
{
    bool required = false;
    std::string error;
    if (!auth.initialSetupRequired(required, error) || !required)
    {
        QMessageBox::critical(nullptr, QStringLiteral("Khởi tạo bị từ chối"),
            error.empty() ? QStringLiteral("Bộ dữ liệu đã có tài khoản; không khởi tạo lại.") : QString::fromStdString(error));
        return false;
    }
    QDialog dialog;
    dialog.setObjectName(QStringLiteral("adminSetup"));
    dialog.setWindowTitle(QStringLiteral("FRAME | Cấp Admin đầu tiên — không phải đăng nhập"));
    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    auto* username = new QLineEdit(&dialog);
    username->setObjectName(QStringLiteral("setupUsername"));
    auto* displayName = new QLineEdit(&dialog);
    displayName->setObjectName(QStringLiteral("setupDisplayName"));
    auto* password = new QLineEdit(&dialog);
    password->setObjectName(QStringLiteral("setupPassword"));
    auto* confirmation = new QLineEdit(&dialog);
    confirmation->setObjectName(QStringLiteral("setupConfirmation"));
    password->setEchoMode(QLineEdit::Password);
    confirmation->setEchoMode(QLineEdit::Password);
    form->addRow(QStringLiteral("&Tên đăng nhập"), username);
    form->addRow(QStringLiteral("Tên &hiển thị"), displayName);
    form->addRow(QStringLiteral("&Mật khẩu"), password);
    form->addRow(QStringLiteral("&Nhập lại mật khẩu"), confirmation);
    layout->addLayout(form);
    auto* canSell = new QCheckBox(QStringLiteral("Cấp riêng quyền bán vé"), &dialog);
    canSell->setObjectName(QStringLiteral("setupCanSell"));
    layout->addWidget(canSell);
    auto* message = new QLabel(&dialog);
    message->setTextFormat(Qt::PlainText);
    message->setWordWrap(true);
    layout->addWidget(message);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        if (password->text() != confirmation->text())
        { message->setText(QStringLiteral("Hai mật khẩu không khớp.")); return; }
        if (!auth.initializeManager(username->text().trimmed().toStdString(), displayName->text().trimmed().toStdString(),
            password->text().toStdString(), error, canSell->isChecked()))
        { message->setText(QString::fromStdString(error)); return; }
        password->clear();
        confirmation->clear();
        dialog.accept();
    });
    return dialog.exec() == QDialog::Accepted;
}

#endif
