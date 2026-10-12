#include "AccountPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVariant>
#include <QVBoxLayout>

AccountPage::AccountPage(cinema::AuthController& controller, QWidget* parent) : QWidget(parent), auth(controller)
{
    setObjectName(QStringLiteral("accountPage"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    auto* explanation = new QLabel(QStringLiteral(
        "Admin quản lý tài khoản và phân quyền. Nhân viên chỉ sử dụng nghiệp vụ được cấp.\n"
        "Quyền bán vé được cấp riêng cho cả hai vai trò; không suy ra từ tên đăng nhập."), this);
    explanation->setWordWrap(true);
    layout->addWidget(explanation);
    auto* actions = new QGridLayout;
    actions->setSpacing(12);
    addButton = new QPushButton(QStringLiteral("Thêm tài khoản"), this);
    addButton->setObjectName(QStringLiteral("accountAdd"));
    addButton->setProperty("primary", QVariant(true));
    editButton = new QPushButton(QStringLiteral("Chỉnh sửa"), this);
    editButton->setObjectName(QStringLiteral("accountEdit"));
    permissionButton = new QPushButton(QStringLiteral("Cấp / Thu hồi bán vé"), this);
    permissionButton->setObjectName(QStringLiteral("accountPermission"));
    statusButton = new QPushButton(QStringLiteral("Khóa / Mở khóa"), this);
    statusButton->setObjectName(QStringLiteral("accountStatus"));
    auto* refresh = new QPushButton(QStringLiteral("Tải lại"), this);
    refresh->setObjectName(QStringLiteral("accountRefresh"));
    actions->addWidget(addButton, 0, 0);
    actions->addWidget(editButton, 0, 1);
    actions->addWidget(refresh, 0, 2);
    actions->addWidget(permissionButton, 1, 0, 1, 2);
    actions->addWidget(statusButton, 1, 2);
    layout->addLayout(actions);
    summary = new QLabel(this);
    summary->setObjectName(QStringLiteral("accountSummary"));
    layout->addWidget(summary);
    errorLabel = new QLabel(this);
    errorLabel->setObjectName(QStringLiteral("accountError"));
    errorLabel->setTextFormat(Qt::PlainText);
    errorLabel->setWordWrap(true);
    errorLabel->hide();
    layout->addWidget(errorLabel);
    table = new QTableWidget(0, 5, this);
    table->setObjectName(QStringLiteral("accountTable"));
    table->setAccessibleName(QStringLiteral("Danh sách tài khoản admin và nhân viên"));
    table->setHorizontalHeaderLabels({QStringLiteral("Tài khoản"), QStringLiteral("Tên hiển thị"),
        QStringLiteral("Vai trò"), QStringLiteral("Bán vé"), QStringLiteral("Trạng thái")});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setMinimumHeight(240);
    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(48);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setMinimumSectionSize(100);
    layout->addWidget(table, 1);
    connect(table, &QTableWidget::itemSelectionChanged, this, [this] { updateActions(); });
    connect(addButton, &QPushButton::clicked, this, [this] { showForm(true); });
    connect(editButton, &QPushButton::clicked, this, [this] { showForm(false); });
    connect(permissionButton, &QPushButton::clicked, this, [this] { toggle(true); });
    connect(statusButton, &QPushButton::clicked, this, [this] { toggle(false); });
    connect(refresh, &QPushButton::clicked, this, [this] { reload(); });
    refreshPermissions();
}

void AccountPage::refreshPermissions()
{
    cinema::LoginSession session;
    std::string error;
    canManage = auth.currentSession(session, error) && session.role == cinema::UserRole::Manager;
    addButton->setEnabled(canManage);
    if (!canManage)
    {
        table->setRowCount(0);
        summary->clear();
        errorLabel->clear();
        errorLabel->hide();
    }
    updateActions();
}

void AccountPage::updateActions()
{
    const bool selected = canManage && table->currentRow() >= 0;
    editButton->setEnabled(selected);
    permissionButton->setEnabled(selected);
    statusButton->setEnabled(selected);
}

void AccountPage::reload()
{
    refreshPermissions();
    errorLabel->clear();
    errorLabel->hide();
    DoublyLinkedList<cinema::UserSummary> users;
    std::string error;
    const bool success = auth.listUsers(users, error);
    table->setRowCount(0);
    if (!success)
    {
        errorLabel->setText(QString::fromStdString(error));
        errorLabel->show();
        return;
    }
    table->setRowCount(users.size());
    int row = 0;
    for (const Node<cinema::UserSummary>* node = users.getHead(); node != nullptr; node = node->next, ++row)
    {
        const auto& user = node->data;
        auto* account = new QTableWidgetItem(QString::fromStdString(user.username));
        account->setData(Qt::UserRole, QString::fromStdString(user.userId));
        table->setItem(row, 0, account);
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(user.displayName)));
        table->setItem(row, 2, new QTableWidgetItem(user.role == cinema::UserRole::Manager
            ? QStringLiteral("Admin (Quản lý)") : QStringLiteral("Nhân viên")));
        table->setItem(row, 3, new QTableWidgetItem(user.canSellTicket ? QStringLiteral("Được cấp") : QStringLiteral("Chưa cấp")));
        table->setItem(row, 4, new QTableWidgetItem(user.status == cinema::RecordStatus::Active
            ? QStringLiteral("Hoạt động") : QStringLiteral("Đã khóa")));
    }
    summary->setText(QStringLiteral("%1 tài khoản · Không hiển thị mật khẩu hoặc hash").arg(users.size()));
    updateActions();
}

bool AccountPage::selectedUser(cinema::UserSummary& user)
{
    const int row = table->currentRow();
    if (row < 0 || table->item(row, 0) == nullptr) return false;
    std::string error;
    if (auth.findUser(table->item(row, 0)->data(Qt::UserRole).toString().toStdString(), user, error)) return true;
    errorLabel->setText(QString::fromStdString(error));
    errorLabel->show();
    return false;
}

void AccountPage::showForm(bool creating)
{
    refreshPermissions();
    if (!canManage) return;
    cinema::UserSummary user;
    if (!creating && !selectedUser(user)) return;
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("accountForm"));
    dialog.setWindowTitle(creating ? QStringLiteral("Thêm tài khoản") : QStringLiteral("Chỉnh sửa tài khoản"));
    dialog.setMinimumWidth(420);
    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    form->setSpacing(10);
    auto* username = new QLineEdit(QString::fromStdString(user.username), &dialog);
    username->setObjectName(QStringLiteral("accountUsername"));
    form->addRow(QStringLiteral("&Tên đăng nhập"), username);
    auto* name = new QLineEdit(QString::fromStdString(user.displayName), &dialog);
    name->setObjectName(QStringLiteral("accountDisplayName"));
    form->addRow(QStringLiteral("Tên &hiển thị"), name);
    auto* role = new QComboBox(&dialog);
    role->setObjectName(QStringLiteral("accountRole"));
    role->addItem(QStringLiteral("Nhân viên"));
    role->addItem(QStringLiteral("Admin (Quản lý)"));
    role->setCurrentIndex(user.role == cinema::UserRole::Manager ? 1 : 0);
    form->addRow(QStringLiteral("&Vai trò"), role);
    auto* password = new QLineEdit(&dialog);
    password->setObjectName(QStringLiteral("accountPassword"));
    password->setEchoMode(QLineEdit::Password);
    form->addRow(creating ? QStringLiteral("&Mật khẩu") : QStringLiteral("Mật khẩu mới (để trống để giữ nguyên)"), password);
    auto* confirmation = new QLineEdit(&dialog);
    confirmation->setObjectName(QStringLiteral("accountConfirmation"));
    confirmation->setEchoMode(QLineEdit::Password);
    form->addRow(QStringLiteral("Nhập &lại mật khẩu"), confirmation);
    auto* status = new QComboBox(&dialog);
    status->setObjectName(QStringLiteral("accountRecordStatus"));
    status->addItem(QStringLiteral("Hoạt động"));
    status->addItem(QStringLiteral("Đã khóa"));
    status->setCurrentIndex(user.status == cinema::RecordStatus::Active ? 0 : 1);
    form->addRow(QStringLiteral("Trạng thá&i"), status);
    layout->addLayout(form);
    auto* sell = new QCheckBox(QStringLiteral("Cấp quyền bán vé"), &dialog);
    sell->setObjectName(QStringLiteral("accountCanSell"));
    sell->setChecked(creating || user.canSellTicket);
    layout->addWidget(sell);
    auto* validation = new QLabel(&dialog);
    validation->setObjectName(QStringLiteral("accountFormError"));
    validation->setTextFormat(Qt::PlainText);
    validation->setWordWrap(true);
    validation->setFocusPolicy(Qt::StrongFocus);
    validation->hide();
    layout->addWidget(validation);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    auto* save = buttons->button(QDialogButtonBox::Save);
    save->setText(QStringLiteral("Lưu tài khoản"));
    save->setObjectName(QStringLiteral("accountSave"));
    save->setProperty("primary", QVariant(true));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        if (password->text() != confirmation->text())
        {
            validation->setText(QStringLiteral("Mật khẩu nhập lại không khớp."));
            validation->show();
            confirmation->setFocus();
            return;
        }
        const cinema::UserRequest request{user.userId, username->text().trimmed().toStdString(),
            name->text().trimmed().toStdString(), password->text().toStdString(),
            role->currentIndex() == 1 ? cinema::UserRole::Manager : cinema::UserRole::Seller,
            sell->isChecked(), status->currentIndex() == 0 ? cinema::RecordStatus::Active : cinema::RecordStatus::Inactive};
        std::string error;
        const bool success = creating ? auth.createUser(request, error) : auth.updateUser(request, error);
        password->clear();
        confirmation->clear();
        if (success) dialog.accept();
        else { validation->setText(QString::fromStdString(error)); validation->show(); validation->setFocus(); }
    });
    username->setFocus();
    if (dialog.exec() == QDialog::Accepted) reload();
}

void AccountPage::toggle(bool permission)
{
    refreshPermissions();
    if (!canManage) return;
    cinema::UserSummary user;
    if (!selectedUser(user)) return;
    QMessageBox confirmation(QMessageBox::Question, QStringLiteral("Xác nhận cập nhật tài khoản"),
        QStringLiteral("%1 tài khoản %2?").arg(permission
            ? (user.canSellTicket ? QStringLiteral("Thu hồi quyền bán vé của") : QStringLiteral("Cấp quyền bán vé cho"))
            : (user.status == cinema::RecordStatus::Active ? QStringLiteral("Khóa") : QStringLiteral("Mở khóa")),
            QString::fromStdString(user.username)), QMessageBox::Yes | QMessageBox::No, this);
    confirmation.setTextFormat(Qt::PlainText);
    confirmation.setDefaultButton(QMessageBox::No);
    if (confirmation.exec() != QMessageBox::Yes) return;
    cinema::UserRequest request{user.userId, user.username, user.displayName, {}, user.role, user.canSellTicket, user.status};
    if (permission) request.canSellTicket = !request.canSellTicket;
    else request.status = user.status == cinema::RecordStatus::Active ? cinema::RecordStatus::Inactive : cinema::RecordStatus::Active;
    std::string error;
    if (auth.updateUser(request, error)) reload();
    else { errorLabel->setText(QString::fromStdString(error)); errorLabel->show(); }
}
