#include "LoginDialog.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(cinema::AuthController& authentication, QWidget* parent)
    : QDialog(parent), auth(authentication)
{
    setWindowTitle(QStringLiteral("FRAME | Đăng nhập"));
    setMinimumWidth(400);
    resize(460, 520);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(16);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    auto* brand = new QLabel(QStringLiteral("FRAME"), this);
    brand->setObjectName(QStringLiteral("brand"));
    layout->addWidget(brand);
    auto* heading = new QLabel(QStringLiteral("Chào mừng trở lại"), this);
    heading->setObjectName(QStringLiteral("heading"));
    layout->addWidget(heading);
    auto* help = new QLabel(QStringLiteral("Admin và nhân viên đăng nhập bằng tài khoản được cấp. "
        "Vai trò được xác định từ tài khoản, không chọn ở màn hình đăng nhập."), this);
    help->setWordWrap(true);
    layout->addWidget(help);

    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    form->setSpacing(8);
    username = new QLineEdit(this);
    username->setObjectName(QStringLiteral("username"));
    username->setMaxLength(128);
    username->setAccessibleName(QStringLiteral("Tên đăng nhập"));
    form->addRow(QStringLiteral("&Tên đăng nhập"), username);
    password = new QLineEdit(this);
    password->setObjectName(QStringLiteral("password"));
    password->setEchoMode(QLineEdit::Password);
    password->setAccessibleName(QStringLiteral("Mật khẩu"));
    form->addRow(QStringLiteral("&Mật khẩu"), password);
    layout->addLayout(form);
    auto* showPassword = new QCheckBox(QStringLiteral("Hiện mật khẩu"), this);
    connect(showPassword, &QCheckBox::toggled, this, [this](bool shown) {
        password->setEchoMode(shown ? QLineEdit::Normal : QLineEdit::Password);
    });
    layout->addWidget(showPassword);
    errorLabel = new QLabel(this);
    errorLabel->setObjectName(QStringLiteral("error"));
    errorLabel->setTextFormat(Qt::PlainText);
    errorLabel->setWordWrap(true);
    errorLabel->setAccessibleName(QStringLiteral("Lỗi đăng nhập"));
    layout->addWidget(errorLabel);
    bool noAccounts = false;
    std::string error;
    if (!auth.initialSetupRequired(noAccounts, error))
        errorLabel->setText(QString::fromStdString(error));
    else if (noAccounts)
        errorLabel->setText(QStringLiteral("Bộ dữ liệu chưa có tài khoản được cấp. "
            "Cấp Admin bằng chế độ --setup-admin hoặc chọn dữ liệu đã có tài khoản; đăng nhập không tạo tài khoản mới."));
    submitButton = new QPushButton(QStringLiteral("Đăng nhập"), this);
    submitButton->setObjectName(QStringLiteral("submit"));
    submitButton->setDefault(true);
    connect(submitButton, &QPushButton::clicked, this, [this] { submit(); });
    layout->addWidget(submitButton);
    username->setFocus();
}

void LoginDialog::submit()
{
    auth.logout();
    errorLabel->clear();
    submitButton->setEnabled(false);
    std::string error;
    const std::string loginName = username->text().trimmed().toStdString();
    const std::string secret = password->text().toStdString();
    cinema::LoginSession session;
    const bool success = auth.login(loginName, secret, session, error);
    password->clear();
    submitButton->setEnabled(true);
    if (success) accept();
    else
    {
        errorLabel->setText(QString::fromStdString(error));
        password->setFocus();
    }
}

void LoginDialog::reject()
{
    auth.logout();
    password->clear();
    QDialog::reject();
}
