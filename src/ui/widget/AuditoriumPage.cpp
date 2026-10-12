#include "AuditoriumPage.h"

#include <algorithm>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

AuditoriumPage::AuditoriumPage(cinema::AuditoriumController& service,
                             cinema::AuthController& authentication, QWidget* parent)
    : QWidget(parent), auditoriums(service), auth(authentication)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 16, 24, 24);
    layout->setSpacing(12);
    auto* filters = new QGridLayout;
    auto* roomLabel = new QLabel(QStringLiteral("Phòng chiếu"), this);
    roomChoice = new QComboBox(this);
    roomChoice->setObjectName(QStringLiteral("auditoriumChoice"));
    roomChoice->setAccessibleName(roomLabel->text());
    roomChoice->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    roomChoice->setMinimumContentsLength(16);
    roomLabel->setBuddy(roomChoice);
    auto* statusLabel = new QLabel(QStringLiteral("Trạng thái phòng"), this);
    statusFilter = new QComboBox(this);
    statusFilter->setObjectName(QStringLiteral("auditoriumFilter"));
    statusFilter->setAccessibleName(statusLabel->text());
    statusFilter->addItems({QStringLiteral("Tất cả"), QStringLiteral("Hoạt động"), QStringLiteral("Ngừng hoạt động")});
    statusLabel->setBuddy(statusFilter);
    auto* refresh = new QPushButton(QStringLiteral("Tải lại"), this);
    refresh->setObjectName(QStringLiteral("auditoriumRefresh"));
    filters->addWidget(roomLabel, 0, 0);
    filters->addWidget(statusLabel, 0, 1);
    filters->addWidget(roomChoice, 1, 0);
    filters->addWidget(statusFilter, 1, 1);
    filters->addWidget(refresh, 2, 0, 1, 2, Qt::AlignLeft);
    addButton = new QPushButton(QStringLiteral("Thêm phòng"), this);
    addButton->setObjectName(QStringLiteral("auditoriumAdd"));
    editButton = new QPushButton(QStringLiteral("Sửa phòng / trạng thái"), this);
    editButton->setObjectName(QStringLiteral("auditoriumEdit"));
    filters->addWidget(addButton, 3, 0);
    filters->addWidget(editButton, 3, 1);
    filters->setColumnStretch(0, 1);
    layout->addLayout(filters);
    summary = new QLabel(this);
    summary->setObjectName(QStringLiteral("auditoriumSummary"));
    summary->setTextFormat(Qt::PlainText);
    summary->setWordWrap(true);
    layout->addWidget(summary);
    errorLabel = new QLabel(this);
    errorLabel->setObjectName(QStringLiteral("error"));
    errorLabel->setTextFormat(Qt::PlainText);
    errorLabel->setWordWrap(true);
    layout->addWidget(errorLabel);
    auto* screen = new QLabel(QStringLiteral("MÀN HÌNH"), this);
    screen->setObjectName(QStringLiteral("auditoriumScreen"));
    screen->setAlignment(Qt::AlignCenter);
    layout->addWidget(screen);
    seatTable = new QTableWidget(this);
    seatTable->setObjectName(QStringLiteral("auditoriumSeats"));
    seatTable->setAccessibleName(QStringLiteral("Sơ đồ ghế; hàng A ở gần màn hình"));
    seatTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    seatTable->setSelectionMode(QAbstractItemView::SingleSelection);
    seatTable->setMinimumHeight(260);
    seatTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    seatTable->horizontalHeader()->setMinimumSectionSize(56);
    seatTable->verticalHeader()->setDefaultSectionSize(48);
    layout->addWidget(seatTable, 1);
    auto* note = new QLabel(QStringLiteral("Ghế thường: mã A01, A02… · Ghế ngừng dùng: mã + [Ngừng].\n"
        "Đây là sơ đồ phòng, chưa phải trạng thái trống/đã bán theo suất chiếu."), this);
    note->setObjectName(QStringLiteral("muted"));
    note->setWordWrap(true);
    layout->addWidget(note);
    connect(roomChoice, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { showRoom(); });
    connect(statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { reload(); });
    connect(refresh, &QPushButton::clicked, this, [this] { reload(); });
    connect(addButton, &QPushButton::clicked, this, [this] { showForm(true); });
    connect(editButton, &QPushButton::clicked, this, [this] { showForm(false); });
    reload();
}

void AuditoriumPage::refreshPermissions()
{
    cinema::LoginSession session;
    std::string error;
    const bool loggedIn = auth.currentSession(session, error);
    const bool manager = loggedIn && session.role == cinema::UserRole::Manager;
    addButton->setVisible(manager);
    editButton->setVisible(manager);
    editButton->setEnabled(manager && roomChoice->currentIndex() >= 0);
    roomChoice->setEnabled(loggedIn);
    statusFilter->setEnabled(loggedIn);
    if (!loggedIn)
    {
        const QSignalBlocker blocker(roomChoice);
        roomChoice->clear();
        seatTable->setRowCount(0);
        seatTable->setColumnCount(0);
        summary->clear();
        errorLabel->setText(QString::fromStdString(error));
        errorLabel->show();
    }
}

void AuditoriumPage::reload()
{
    refreshPermissions();
    const QString previousId = roomChoice->currentData().toString();
    std::optional<cinema::RecordStatus> status;
    if (statusFilter->currentIndex() == 1) status = cinema::RecordStatus::Active;
    if (statusFilter->currentIndex() == 2) status = cinema::RecordStatus::Inactive;
    DoublyLinkedList<cinema::Auditorium> rooms;
    std::string error;
    const bool success = auditoriums.list(status, rooms, error);
    {
        const QSignalBlocker blocker(roomChoice);
        roomChoice->clear();
        for (const Node<cinema::Auditorium>* node = rooms.getHead(); node != nullptr; node = node->next)
        {
            const auto& room = node->data;
            roomChoice->addItem(QStringLiteral("%1 · %2").arg(QString::fromStdString(room.auditoriumId),
                QString::fromStdString(room.name)), QString::fromStdString(room.auditoriumId));
        }
        const int previousIndex = roomChoice->findData(previousId);
        if (previousIndex >= 0) roomChoice->setCurrentIndex(previousIndex);
    }
    showRoom();
    if (!success)
    {
        errorLabel->setText(QString::fromStdString(error));
        errorLabel->show();
    }
}

void AuditoriumPage::showRoom()
{
    refreshPermissions();
    seatTable->setRowCount(0);
    seatTable->setColumnCount(0);
    errorLabel->hide();
    if (roomChoice->currentIndex() < 0)
    {
        summary->setText(QStringLiteral("Không có phòng phù hợp. Bộ dữ liệu mới không tự tạo phòng hoặc ghế mẫu."));
        return;
    }
    cinema::Auditorium room;
    DoublyLinkedList<cinema::Seat> seats;
    std::string error;
    if (!auditoriums.details(roomChoice->currentData().toString().toStdString(), room, seats, error))
    {
        summary->clear();
        errorLabel->setText(QString::fromStdString(error));
        errorLabel->show();
        return;
    }
    seatTable->setRowCount(room.rows);
    seatTable->setColumnCount(room.columns);
    for (int row = 0; row < room.rows; ++row)
        seatTable->setVerticalHeaderItem(row, new QTableWidgetItem(QString(QChar('A' + row))));
    for (int column = 0; column < room.columns; ++column)
        seatTable->setHorizontalHeaderItem(column, new QTableWidgetItem(QString::number(column + 1)));
    int active = 0;
    for (const Node<cinema::Seat>* node = seats.getHead(); node != nullptr; node = node->next)
    {
        const auto& seat = node->data;
        if (seat.rowLabel.empty()) continue;
        const int row = seat.rowLabel[0] - 'A';
        const int column = seat.seatNumber - 1;
        if (row < 0 || row >= room.rows || column < 0 || column >= room.columns) continue;
        const bool enabled = seat.status == cinema::RecordStatus::Active;
        if (enabled) ++active;
        auto* item = new QTableWidgetItem(QString::fromStdString(seat.seatCode)
            + (enabled ? QString() : QStringLiteral(" [Ngừng]")));
        item->setTextAlignment(Qt::AlignCenter);
        item->setData(Qt::UserRole, QString::fromStdString(seat.seatId));
        item->setToolTip(QStringLiteral("%1 · Ghế thường · %2").arg(QString::fromStdString(seat.seatId),
            enabled ? QStringLiteral("Hoạt động") : QStringLiteral("Ngừng sử dụng")));
        seatTable->setItem(row, column, item);
    }
    summary->setText(QStringLiteral("%1 · %2 hàng × %3 cột · %4/%5 ghế hoạt động · %6")
        .arg(QString::fromStdString(room.name)).arg(room.rows).arg(room.columns).arg(active).arg(room.capacity)
        .arg(room.status == cinema::RecordStatus::Active ? QStringLiteral("Hoạt động") : QStringLiteral("Ngừng hoạt động")));
}

void AuditoriumPage::showForm(bool creating)
{
    cinema::LoginSession session;
    std::string error;
    if (!auth.currentSession(session, error) || session.role != cinema::UserRole::Manager) return;
    cinema::Auditorium room;
    DoublyLinkedList<cinema::Seat> seats;
    if (!creating && !auditoriums.details(roomChoice->currentData().toString().toStdString(), room, seats, error))
    { errorLabel->setText(QString::fromStdString(error)); errorLabel->show(); return; }
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("auditoriumForm"));
    dialog.setWindowTitle(creating ? QStringLiteral("Thêm phòng chiếu") : QStringLiteral("Sửa phòng chiếu"));
    dialog.setMinimumWidth(420);
    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    auto* id = new QLineEdit(QString::fromStdString(room.auditoriumId), &dialog);
    id->setObjectName(QStringLiteral("auditoriumId"));
    id->setReadOnly(!creating);
    auto* name = new QLineEdit(QString::fromStdString(room.name), &dialog);
    name->setObjectName(QStringLiteral("auditoriumName"));
    auto* rows = new QSpinBox(&dialog);
    rows->setObjectName(QStringLiteral("auditoriumRows"));
    rows->setRange(1, 26);
    rows->setValue(creating ? 6 : room.rows);
    auto* columns = new QSpinBox(&dialog);
    columns->setObjectName(QStringLiteral("auditoriumColumns"));
    columns->setRange(1, creating ? 100 : std::max(100, room.columns));
    columns->setValue(creating ? 8 : room.columns);
    auto* status = new QComboBox(&dialog);
    status->setObjectName(QStringLiteral("auditoriumRecordStatus"));
    status->addItems({QStringLiteral("Hoạt động"), QStringLiteral("Ngừng hoạt động")});
    status->setCurrentIndex(room.status == cinema::RecordStatus::Active ? 0 : 1);
    form->addRow(QStringLiteral("&Mã phòng *"), id);
    form->addRow(QStringLiteral("&Tên phòng *"), name);
    form->addRow(QStringLiteral("Số &hàng * (A–Z)"), rows);
    form->addRow(QStringLiteral("Số &cột *"), columns);
    form->addRow(QStringLiteral("Trạng thá&i *"), status);
    layout->addLayout(form);
    auto* capacity = new QLabel(&dialog);
    const auto updateCapacity = [=] { capacity->setText(QStringLiteral("Sức chứa: %1 ghế STANDARD. * Bắt buộc.")
        .arg(rows->value() * columns->value())); };
    connect(rows, QOverload<int>::of(&QSpinBox::valueChanged), &dialog, updateCapacity);
    connect(columns, QOverload<int>::of(&QSpinBox::valueChanged), &dialog, updateCapacity);
    updateCapacity();
    layout->addWidget(capacity);
    auto* validation = new QLabel(&dialog);
    validation->setObjectName(QStringLiteral("auditoriumFormError"));
    validation->setTextFormat(Qt::PlainText);
    validation->setWordWrap(true);
    layout->addWidget(validation);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setObjectName(QStringLiteral("auditoriumSave"));
    buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("Lưu phòng"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        const cinema::Auditorium candidate{id->text().trimmed().toStdString(), name->text().trimmed().toStdString(),
            rows->value(), columns->value(), 0, status->currentIndex() == 0 ? cinema::RecordStatus::Active : cinema::RecordStatus::Inactive};
        if (auditoriums.saveGrid(candidate, creating, error)) dialog.accept();
        else validation->setText(QString::fromStdString(error));
    });
    if (dialog.exec() == QDialog::Accepted) reload();
}
