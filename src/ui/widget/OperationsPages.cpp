#include "OperationsPages.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>

namespace
{
QString statusText(cinema::ScreeningStatus status)
{
    switch (status)
    {
    case cinema::ScreeningStatus::Draft: return QStringLiteral("Nháp");
    case cinema::ScreeningStatus::Open: return QStringLiteral("Mở bán");
    case cinema::ScreeningStatus::Closed: return QStringLiteral("Đóng bán");
    case cinema::ScreeningStatus::Completed: return QStringLiteral("Hoàn tất");
    }
    return QStringLiteral("Không hợp lệ");
}

QTableWidget* makeTable(const QStringList& headers, const QString& name, QWidget* parent)
{
    auto* table = new QTableWidget(0, headers.size(), parent);
    table->setObjectName(name);
    table->setAccessibleName(name);
    table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(44);
    table->setMinimumHeight(220);
    return table;
}

void rowValues(QTableWidget* table, int row, const QStringList& values)
{
    for (int column = 0; column < values.size(); ++column)
        table->setItem(row, column, new QTableWidgetItem(values[column]));
}

QLabel* makeError(QWidget* parent)
{
    auto* label = new QLabel(parent);
    label->setObjectName(QStringLiteral("error"));
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    return label;
}

void loadFilters(cinema::FilmController& films, cinema::AuditoriumController& rooms,
                 QComboBox* filmChoice, QComboBox* roomChoice, bool activeOnly, std::string& error)
{
    const QString previousFilm = filmChoice->currentData().toString();
    const QString previousRoom = roomChoice->currentData().toString();
    const QSignalBlocker filmBlock(filmChoice), roomBlock(roomChoice);
    filmChoice->clear();
    roomChoice->clear();
    filmChoice->addItem(QStringLiteral("Tất cả phim"), QString());
    roomChoice->addItem(QStringLiteral("Tất cả phòng"), QString());
    DoublyLinkedList<cinema::Film> filmRecords;
    DoublyLinkedList<cinema::Auditorium> roomRecords;
    const auto status = activeOnly ? std::optional<cinema::RecordStatus>(cinema::RecordStatus::Active) : std::nullopt;
    if (!films.list({}, status, filmRecords, error) || !rooms.list(status, roomRecords, error)) return;
    for (const Node<cinema::Film>* node = filmRecords.getHead(); node != nullptr; node = node->next)
        filmChoice->addItem(QString::fromStdString(node->data.title), QString::fromStdString(node->data.filmId));
    for (const Node<cinema::Auditorium>* node = roomRecords.getHead(); node != nullptr; node = node->next)
        roomChoice->addItem(QString::fromStdString(node->data.name), QString::fromStdString(node->data.auditoriumId));
    if (filmChoice->findData(previousFilm) >= 0) filmChoice->setCurrentIndex(filmChoice->findData(previousFilm));
    if (roomChoice->findData(previousRoom) >= 0) roomChoice->setCurrentIndex(roomChoice->findData(previousRoom));
}

QString saleText(const cinema::SaleDetails& sale)
{
    const auto& transaction = sale.receipt.transaction;
    QString text = QStringLiteral("Giao dịch: %1\nPhim: %2 · Độ tuổi: %3\nSuất: %4\nPhòng: %5\nBắt đầu: %6\nKết thúc: %7\nNgười bán: %8\nThời điểm: %9\nThanh toán: %10\nTổng giao dịch: %11 VND\n\n")
        .arg(QString::fromStdString(transaction.transactionId), QString::fromStdString(sale.show.film.title),
             QString::fromStdString(sale.show.film.ageRating), QString::fromStdString(sale.show.screening.screeningId),
             QString::fromStdString(sale.show.auditorium.name), QString::fromStdString(sale.show.screening.startTime),
             QString::fromStdString(sale.show.screening.endTime), QString::fromStdString(sale.sellerName),
             QString::fromStdString(transaction.createdAt))
        .arg(transaction.paymentStatus == cinema::PaymentStatus::Confirmed ? QStringLiteral("Đã xác nhận") : QStringLiteral("Chưa xác nhận"))
        .arg(transaction.totalAmount);
    const Node<cinema::Seat>* seat = sale.seats.getHead();
    for (const Node<cinema::Ticket>* ticket = sale.receipt.tickets.getHead(); ticket != nullptr; ticket = ticket->next)
    {
        text += QStringLiteral("Vé: %1 · Ghế: %2 · %3 VND · %4\n")
            .arg(QString::fromStdString(ticket->data.ticketId), seat == nullptr ? QStringLiteral("?") : QString::fromStdString(seat->data.seatCode))
            .arg(ticket->data.unitPrice)
            .arg(transaction.status == cinema::TransactionStatus::Completed ? QStringLiteral("Đã phát hành") : QStringLiteral("Chờ xác nhận"));
        if (seat != nullptr) seat = seat->next;
    }
    return text;
}
}

ScreeningPage::ScreeningPage(cinema::OperationsController& controller, cinema::FilmController& filmController,
    cinema::AuditoriumController& roomController, cinema::AuthController& authentication, QWidget* parent)
    : QWidget(parent), operations(controller), films(filmController), rooms(roomController), auth(authentication)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 16, 24, 24);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    filmFilter = new QComboBox(this);
    filmFilter->setObjectName(QStringLiteral("screeningFilmFilter"));
    roomFilter = new QComboBox(this);
    roomFilter->setObjectName(QStringLiteral("screeningRoomFilter"));
    dateFilter = new QLineEdit(this);
    dateFilter->setObjectName(QStringLiteral("screeningDateFilter"));
    dateFilter->setPlaceholderText(QStringLiteral("YYYY-MM-DD; để trống xem mọi ngày"));
    statusFilter = new QComboBox(this);
    statusFilter->setObjectName(QStringLiteral("screeningStatusFilter"));
    statusFilter->addItems({QStringLiteral("Tất cả trạng thái"), QStringLiteral("Nháp"), QStringLiteral("Mở bán"), QStringLiteral("Đóng bán"), QStringLiteral("Hoàn tất")});
    form->addRow(QStringLiteral("&Phim"), filmFilter);
    form->addRow(QStringLiteral("Phò&ng"), roomFilter);
    form->addRow(QStringLiteral("N&gày"), dateFilter);
    form->addRow(QStringLiteral("Trạng thá&i"), statusFilter);
    layout->addLayout(form);
    auto* actions = new QGridLayout;
    auto* refresh = new QPushButton(QStringLiteral("Tải lại / Tìm"), this);
    refresh->setObjectName(QStringLiteral("screeningRefresh"));
    addButton = new QPushButton(QStringLiteral("Thêm suất chiếu"), this);
    addButton->setObjectName(QStringLiteral("screeningAdd"));
    openButton = new QPushButton(QStringLiteral("Mở bán"), this);
    closeButton = new QPushButton(QStringLiteral("Đóng bán"), this);
    completeButton = new QPushButton(QStringLiteral("Hoàn tất"), this);
    openButton->setObjectName(QStringLiteral("screeningOpen"));
    closeButton->setObjectName(QStringLiteral("screeningClose"));
    completeButton->setObjectName(QStringLiteral("screeningComplete"));
    actions->addWidget(refresh, 0, 0);
    actions->addWidget(addButton, 0, 1);
    actions->addWidget(openButton, 1, 0);
    actions->addWidget(closeButton, 1, 1);
    actions->addWidget(completeButton, 2, 0);
    layout->addLayout(actions);
    errorLabel = makeError(this);
    layout->addWidget(errorLabel);
    table = makeTable({QStringLiteral("Mã suất"), QStringLiteral("Phim"), QStringLiteral("Phòng"), QStringLiteral("Bắt đầu"),
        QStringLiteral("Kết thúc"), QStringLiteral("Phòng sẵn sàng"), QStringLiteral("Giá VND"), QStringLiteral("Trạng thái")},
        QStringLiteral("screeningTable"), this);
    layout->addWidget(table, 1);
    connect(refresh, &QPushButton::clicked, this, [this] { reload(); });
    connect(dateFilter, &QLineEdit::returnPressed, this, [this] { reload(); });
    connect(addButton, &QPushButton::clicked, this, [this] { create(); });
    connect(openButton, &QPushButton::clicked, this, [this] { changeStatus(cinema::ScreeningStatus::Open); });
    connect(closeButton, &QPushButton::clicked, this, [this] { changeStatus(cinema::ScreeningStatus::Closed); });
    connect(completeButton, &QPushButton::clicked, this, [this] { changeStatus(cinema::ScreeningStatus::Completed); });
    connect(table, &QTableWidget::itemSelectionChanged, this, [this] { refreshPermissions(); });
    reload();
}

void ScreeningPage::refreshPermissions()
{
    cinema::LoginSession session;
    std::string error;
    const bool loggedIn = auth.currentSession(session, error);
    const bool manager = loggedIn && session.role == cinema::UserRole::Manager;
    addButton->setVisible(manager);
    for (auto* button : {openButton, closeButton, completeButton})
    { button->setVisible(manager); button->setEnabled(manager && table->currentRow() >= 0); }
    if (!loggedIn) table->setRowCount(0);
}

void ScreeningPage::reload()
{
    std::string error;
    loadFilters(films, rooms, filmFilter, roomFilter, false, error);
    DoublyLinkedList<cinema::ScreeningDetails> records;
    std::optional<cinema::ScreeningStatus> status;
    if (statusFilter->currentIndex() > 0) status = static_cast<cinema::ScreeningStatus>(statusFilter->currentIndex() - 1);
    const bool success = error.empty() && operations.screenings(filmFilter->currentData().toString().toStdString(),
        roomFilter->currentData().toString().toStdString(), dateFilter->text().trimmed().toStdString(), status, false, records, error);
    table->setRowCount(0);
    if (success)
    {
        table->setRowCount(records.size());
        int row = 0;
        for (const Node<cinema::ScreeningDetails>* node = records.getHead(); node != nullptr; node = node->next, ++row)
        {
            const auto& show = node->data.screening;
            rowValues(table, row, {QString::fromStdString(show.screeningId), QString::fromStdString(node->data.film.title),
                QString::fromStdString(node->data.auditorium.name), QString::fromStdString(show.startTime), QString::fromStdString(show.endTime),
                QString::fromStdString(show.roomAvailableUntil), QString::number(show.ticketPrice), statusText(show.status)});
        }
    }
    errorLabel->setText(QString::fromStdString(error));
    refreshPermissions();
}

void ScreeningPage::changeStatus(cinema::ScreeningStatus status)
{
    if (table->currentRow() < 0) return;
    std::string error;
    if (operations.changeStatus(table->item(table->currentRow(), 0)->text().toStdString(), status, error)) reload();
    else errorLabel->setText(QString::fromStdString(error));
}

void ScreeningPage::create()
{
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("screeningForm"));
    dialog.setWindowTitle(QStringLiteral("Tạo suất chiếu"));
    dialog.setMinimumWidth(440);
    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    auto* id = new QLineEdit(&dialog);
    id->setObjectName(QStringLiteral("screeningId"));
    id->setPlaceholderText(QStringLiteral("Để trống để tự sinh mã"));
    auto* film = new QComboBox(&dialog);
    film->setObjectName(QStringLiteral("screeningFilm"));
    auto* room = new QComboBox(&dialog);
    room->setObjectName(QStringLiteral("screeningRoom"));
    std::string error;
    loadFilters(films, rooms, film, room, true, error);
    film->removeItem(0);
    room->removeItem(0);
    auto* start = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), &dialog);
    start->setObjectName(QStringLiteral("screeningStart"));
    start->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    start->setCalendarPopup(true);
    auto* price = new QSpinBox(&dialog);
    price->setObjectName(QStringLiteral("screeningPrice"));
    price->setRange(1, 1000000000);
    price->setValue(80000);
    price->setSuffix(QStringLiteral(" VND"));
    auto* status = new QComboBox(&dialog);
    status->setObjectName(QStringLiteral("screeningNewStatus"));
    status->addItems({QStringLiteral("Nháp"), QStringLiteral("Mở bán")});
    form->addRow(QStringLiteral("&Mã suất"), id);
    form->addRow(QStringLiteral("&Phim *"), film);
    form->addRow(QStringLiteral("Phò&ng *"), room);
    form->addRow(QStringLiteral("&Bắt đầu *"), start);
    form->addRow(QStringLiteral("&Giá vé *"), price);
    form->addRow(QStringLiteral("Trạng thá&i *"), status);
    layout->addLayout(form);
    auto* help = new QLabel(QStringLiteral("* Bắt buộc. Kết thúc = bắt đầu + thời lượng phim. Phòng nghỉ thêm 15 phút."), &dialog);
    help->setWordWrap(true);
    layout->addWidget(help);
    auto* validation = makeError(&dialog);
    validation->setText(QString::fromStdString(error));
    layout->addWidget(validation);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Save)->setObjectName(QStringLiteral("screeningSave"));
    buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("Lưu suất chiếu"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        cinema::Screening result;
        if (operations.createScreening({id->text().trimmed().toStdString(), film->currentData().toString().toStdString(),
            room->currentData().toString().toStdString(), start->dateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")).toStdString(),
            price->value(), status->currentIndex() == 1}, result, error)) dialog.accept();
        else validation->setText(QString::fromStdString(error));
    });
    if (dialog.exec() == QDialog::Accepted) reload();
}

SalePage::SalePage(cinema::OperationsController& controller, cinema::FilmController& filmController,
    cinema::AuditoriumController& roomController, cinema::AuthController& authentication, QWidget* parent)
    : QWidget(parent), operations(controller), films(filmController), rooms(roomController), auth(authentication)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 16, 24, 24);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    filmFilter = new QComboBox(this);
    filmFilter->setObjectName(QStringLiteral("saleFilmFilter"));
    roomFilter = new QComboBox(this);
    roomFilter->setObjectName(QStringLiteral("saleRoomFilter"));
    dateFilter = new QLineEdit(this);
    dateFilter->setObjectName(QStringLiteral("saleDateFilter"));
    dateFilter->setPlaceholderText(QStringLiteral("YYYY-MM-DD; để trống xem mọi ngày"));
    showChoice = new QComboBox(this);
    showChoice->setObjectName(QStringLiteral("saleScreening"));
    showChoice->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    showChoice->setMinimumContentsLength(20);
    form->addRow(QStringLiteral("&Phim"), filmFilter);
    form->addRow(QStringLiteral("Phò&ng"), roomFilter);
    form->addRow(QStringLiteral("N&gày"), dateFilter);
    form->addRow(QStringLiteral("&Suất đang mở bán"), showChoice);
    layout->addLayout(form);
    auto* actions = new QGridLayout;
    auto* refresh = new QPushButton(QStringLiteral("Tải lại / Tìm suất"), this);
    refresh->setObjectName(QStringLiteral("saleRefresh"));
    previewButton = new QPushButton(QStringLiteral("Kiểm tra tiền / Thanh toán"), this);
    previewButton->setObjectName(QStringLiteral("salePreview"));
    cancelButton = new QPushButton(QStringLiteral("Hủy lựa chọn"), this);
    cancelButton->setObjectName(QStringLiteral("saleCancel"));
    actions->addWidget(refresh, 0, 0);
    actions->addWidget(cancelButton, 0, 1);
    actions->addWidget(previewButton, 1, 0, 1, 2);
    layout->addLayout(actions);
    errorLabel = makeError(this);
    summary = new QLabel(this);
    summary->setTextFormat(Qt::PlainText);
    summary->setWordWrap(true);
    layout->addWidget(errorLabel);
    layout->addWidget(summary);
    auto* screen = new QLabel(QStringLiteral("MÀN HÌNH — HÀNG A Ở GẦN MÀN HÌNH"), this);
    screen->setObjectName(QStringLiteral("auditoriumScreen"));
    screen->setAlignment(Qt::AlignCenter);
    layout->addWidget(screen);
    seats = new QTableWidget(this);
    seats->setObjectName(QStringLiteral("saleSeats"));
    seats->setAccessibleName(QStringLiteral("Chọn một hoặc nhiều ghế trống; Ctrl hoặc Shift để chọn nhiều"));
    seats->setEditTriggers(QAbstractItemView::NoEditTriggers);
    seats->setSelectionMode(QAbstractItemView::MultiSelection);
    seats->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    seats->horizontalHeader()->setMinimumSectionSize(64);
    seats->verticalHeader()->setDefaultSectionSize(48);
    seats->setMinimumHeight(260);
    layout->addWidget(seats, 1);
    auto* legend = new QLabel(QStringLiteral("Trống: có thể chọn · Đã bán: không thể chọn · Không dùng: vị trí không bán.\n"
        "Chọn ghế chưa giữ ghế. Nhân viên kiểm tra độ tuổi thực tế trước khi xác nhận tiền mặt."), this);
    legend->setWordWrap(true);
    layout->addWidget(legend);
    connect(refresh, &QPushButton::clicked, this, [this] { reload(); });
    connect(showChoice, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { showSeats(); });
    connect(dateFilter, &QLineEdit::returnPressed, this, [this] { reload(); });
    connect(previewButton, &QPushButton::clicked, this, [this] { checkout(); });
    connect(cancelButton, &QPushButton::clicked, this, [this] { seats->clearSelection(); errorLabel->clear(); });
    connect(seats, &QTableWidget::itemSelectionChanged, this, [this] { refreshPermissions(); });
    reload();
}

void SalePage::refreshPermissions()
{
    cinema::LoginSession session;
    std::string error;
    const bool loggedIn = auth.currentSession(session, error);
    seats->setEnabled(loggedIn && session.canSellTicket);
    previewButton->setEnabled(loggedIn && session.canSellTicket && !seats->selectedItems().isEmpty());
    if (!loggedIn) { seats->setRowCount(0); summary->clear(); }
}

void SalePage::reload()
{
    std::string error;
    loadFilters(films, rooms, filmFilter, roomFilter, true, error);
    DoublyLinkedList<cinema::ScreeningDetails> records;
    const bool success = error.empty() && operations.screenings(filmFilter->currentData().toString().toStdString(),
        roomFilter->currentData().toString().toStdString(), dateFilter->text().trimmed().toStdString(), std::nullopt, true, records, error);
    const QString previous = showChoice->currentData().toString();
    {
        const QSignalBlocker blocker(showChoice);
        showChoice->clear();
        if (success)
            for (const Node<cinema::ScreeningDetails>* node = records.getHead(); node != nullptr; node = node->next)
            {
                const auto& details = node->data;
                showChoice->addItem(QStringLiteral("%1 · %2 · %3 · %4 VND")
                    .arg(QString::fromStdString(details.film.title), QString::fromStdString(details.auditorium.name),
                         QString::fromStdString(details.screening.startTime)).arg(details.screening.ticketPrice),
                    QString::fromStdString(details.screening.screeningId));
            }
        if (showChoice->findData(previous) >= 0) showChoice->setCurrentIndex(showChoice->findData(previous));
    }
    showSeats();
    if (!success) errorLabel->setText(QString::fromStdString(error));
}

void SalePage::showSeats()
{
    seats->setRowCount(0);
    seats->setColumnCount(0);
    refreshPermissions();
    summary->setText(QStringLiteral("Không có suất phù hợp đang mở bán và chưa bắt đầu."));
    errorLabel->clear();
    if (showChoice->currentIndex() < 0) { refreshPermissions(); return; }
    DoublyLinkedList<cinema::ScreeningDetails> records;
    Matrix2D map;
    std::string error;
    const std::string id = showChoice->currentData().toString().toStdString();
    if (!operations.screenings({}, {}, {}, std::nullopt, true, records, error) || !operations.seatMap(id, map, error))
    { errorLabel->setText(QString::fromStdString(error)); return; }
    cinema::ScreeningDetails selected;
    for (const Node<cinema::ScreeningDetails>* node = records.getHead(); node != nullptr; node = node->next)
        if (node->data.screening.screeningId == id) selected = node->data;
    if (selected.screening.screeningId.empty()) { errorLabel->setText(QStringLiteral("Suất không còn mở bán; hãy tải lại.")); return; }
    cinema::Auditorium room;
    DoublyLinkedList<cinema::Seat> recordsSeats;
    if (!rooms.details(selected.auditorium.auditoriumId, room, recordsSeats, error))
    { errorLabel->setText(QString::fromStdString(error)); return; }
    seats->setRowCount(room.rows);
    seats->setColumnCount(room.columns);
    for (int row = 0; row < room.rows; ++row)
        seats->setVerticalHeaderItem(row, new QTableWidgetItem(QString(QChar('A' + row))));
    for (const Node<cinema::Seat>* node = recordsSeats.getHead(); node != nullptr; node = node->next)
    {
        const int row = node->data.rowLabel[0] - 'A', column = node->data.seatNumber - 1;
        const char state = map.get(row, column);
        auto* item = new QTableWidgetItem(QString::fromStdString(node->data.seatCode) + (state == 'O'
            ? QStringLiteral("\nTrống") : state == 'X' ? QStringLiteral("\nĐã bán") : QStringLiteral("\nKhông dùng")));
        item->setData(Qt::UserRole, QString::fromStdString(node->data.seatId));
        item->setTextAlignment(Qt::AlignCenter);
        if (state != 'O') item->setFlags(Qt::ItemIsEnabled);
        seats->setItem(row, column, item);
    }
    summary->setText(QStringLiteral("%1 · %2 · %3 → %4 · %5 VND/ghế · Độ tuổi: %6")
        .arg(QString::fromStdString(selected.film.title), QString::fromStdString(room.name),
            QString::fromStdString(selected.screening.startTime), QString::fromStdString(selected.screening.endTime))
        .arg(selected.screening.ticketPrice).arg(QString::fromStdString(selected.film.ageRating)));
    refreshPermissions();
}

void SalePage::checkout()
{
    cinema::SaleRequest request;
    request.screeningId = showChoice->currentData().toString().toStdString();
    for (auto* item : seats->selectedItems()) request.seatIds.addLast(item->data(Qt::UserRole).toString().toStdString());
    cinema::SaleDetails preview;
    std::string error;
    if (!operations.preview(request, preview, error)) { errorLabel->setText(QString::fromStdString(error)); return; }
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("saleConfirmation"));
    dialog.setWindowTitle(QStringLiteral("Kiểm tra vé và tiền mặt"));
    dialog.resize(560, 540);
    auto* layout = new QVBoxLayout(&dialog);
    auto* text = new QTextEdit(&dialog);
    text->setObjectName(QStringLiteral("salePreviewDetails"));
    text->setReadOnly(true);
    text->setPlainText(saleText(preview));
    layout->addWidget(text, 1);
    auto* paid = new QCheckBox(QStringLiteral("Tôi đã nhận đủ %1 VND tiền mặt").arg(preview.receipt.transaction.totalAmount), &dialog);
    paid->setObjectName(QStringLiteral("salePaid"));
    layout->addWidget(paid);
    auto* validation = makeError(&dialog);
    layout->addWidget(validation);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    auto* submit = buttons->button(QDialogButtonBox::Ok);
    submit->setText(QStringLiteral("Xác nhận và phát hành vé"));
    submit->setObjectName(QStringLiteral("saleConfirm"));
    submit->setEnabled(false);
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy, không bán vé"));
    layout->addWidget(buttons);
    connect(paid, &QCheckBox::toggled, submit, &QPushButton::setEnabled);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        request.paymentConfirmed = paid->isChecked();
        submit->setEnabled(false);
        cinema::SaleReceipt receipt;
        if (operations.sell(request, receipt, error))
        {
            preview.receipt = receipt;
            dialog.accept();
        }
        else { validation->setText(QString::fromStdString(error)); submit->setEnabled(paid->isChecked()); }
    });
    const bool sold = dialog.exec() == QDialog::Accepted;
    seats->clearSelection();
    if (!sold) return;
    reload();
    QDialog receiptDialog(this);
    receiptDialog.setObjectName(QStringLiteral("saleReceipt"));
    receiptDialog.setWindowTitle(QStringLiteral("Vé đã phát hành — dữ liệu đã lưu"));
    receiptDialog.resize(600, 560);
    auto* receiptLayout = new QVBoxLayout(&receiptDialog);
    auto* receiptText = new QTextEdit(&receiptDialog);
    receiptText->setObjectName(QStringLiteral("saleReceiptDetails"));
    receiptText->setReadOnly(true);
    receiptText->setPlainText(saleText(preview));
    receiptLayout->addWidget(receiptText);
    auto* close = new QDialogButtonBox(QDialogButtonBox::Close, &receiptDialog);
    close->button(QDialogButtonBox::Close)->setText(QStringLiteral("Đóng"));
    receiptLayout->addWidget(close);
    connect(close, &QDialogButtonBox::rejected, &receiptDialog, &QDialog::reject);
    receiptDialog.exec();
}

LookupPage::LookupPage(cinema::OperationsController& controller, QWidget* parent)
    : QWidget(parent), operations(controller)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 16, 24, 24);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    type = new QComboBox(this);
    type->addItems({QStringLiteral("Mã giao dịch"), QStringLiteral("Mã vé")});
    type->setObjectName(QStringLiteral("lookupType"));
    idInput = new QLineEdit(this);
    idInput->setObjectName(QStringLiteral("lookupId"));
    form->addRow(QStringLiteral("&Tra cứu theo"), type);
    form->addRow(QStringLiteral("&Mã *"), idInput);
    layout->addLayout(form);
    auto* search = new QPushButton(QStringLiteral("Tra cứu"), this);
    search->setObjectName(QStringLiteral("lookupSearch"));
    layout->addWidget(search);
    errorLabel = makeError(this);
    layout->addWidget(errorLabel);
    result = new QTextEdit(this);
    result->setObjectName(QStringLiteral("lookupResult"));
    result->setReadOnly(true);
    result->setMinimumHeight(260);
    layout->addWidget(result, 1);
    const auto lookup = [this] {
        cinema::SaleDetails details;
        std::string error;
        result->clear();
        if (operations.lookup(idInput->text().trimmed().toStdString(), type->currentIndex() == 1, details, error))
            result->setPlainText(saleText(details));
        errorLabel->setText(QString::fromStdString(error));
    };
    connect(search, &QPushButton::clicked, this, lookup);
    connect(idInput, &QLineEdit::returnPressed, this, lookup);
}

void LookupPage::clear() { result->clear(); idInput->clear(); errorLabel->clear(); }

ReportPage::ReportPage(cinema::OperationsController& controller, QWidget* parent)
    : QWidget(parent), operations(controller)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 16, 24, 24);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    from = new QDateEdit(QDate::currentDate().addMonths(-1), this);
    to = new QDateEdit(QDate::currentDate(), this);
    from->setObjectName(QStringLiteral("reportFrom"));
    to->setObjectName(QStringLiteral("reportTo"));
    for (auto* field : {from, to}) { field->setCalendarPopup(true); field->setDisplayFormat(QStringLiteral("yyyy-MM-dd")); }
    filmFilter = new QComboBox(this);
    filmFilter->setObjectName(QStringLiteral("reportFilm"));
    screeningFilter = new QComboBox(this);
    screeningFilter->setObjectName(QStringLiteral("reportScreening"));
    form->addRow(QStringLiteral("&Từ ngày bán"), from);
    form->addRow(QStringLiteral("&Đến ngày bán"), to);
    form->addRow(QStringLiteral("&Phim"), filmFilter);
    form->addRow(QStringLiteral("&Suất"), screeningFilter);
    layout->addLayout(form);
    auto* refresh = new QPushButton(QStringLiteral("Xem báo cáo"), this);
    refresh->setObjectName(QStringLiteral("reportRefresh"));
    layout->addWidget(refresh);
    errorLabel = makeError(this);
    layout->addWidget(errorLabel);
    summary = new QLabel(this);
    summary->setObjectName(QStringLiteral("reportSummary"));
    summary->setWordWrap(true);
    layout->addWidget(summary);
    table = makeTable({QStringLiteral("Mã phim"), QStringLiteral("Phim"), QStringLiteral("Số vé"), QStringLiteral("Doanh thu VND")},
        QStringLiteral("reportTable"), this);
    layout->addWidget(table);
    auto* note = new QLabel(QStringLiteral("Lấp đầy tính trên toàn bộ vé đã bán của suất, không giới hạn theo ngày bán."), this);
    note->setWordWrap(true);
    layout->addWidget(note);
    occupancy = makeTable({QStringLiteral("Suất"), QStringLiteral("Phim"), QStringLiteral("Phòng"), QStringLiteral("Ghế đã bán"),
        QStringLiteral("Sức chứa"), QStringLiteral("Lấp đầy %")}, QStringLiteral("reportOccupancy"), this);
    layout->addWidget(occupancy);
    connect(refresh, &QPushButton::clicked, this, [this] { run(); });
}

void ReportPage::clear() { summary->clear(); table->setRowCount(0); occupancy->setRowCount(0); errorLabel->clear(); }

void ReportPage::reload()
{
    clear();
    const QString previousFilm = filmFilter->currentData().toString(), previousShow = screeningFilter->currentData().toString();
    filmFilter->clear();
    screeningFilter->clear();
    filmFilter->addItem(QStringLiteral("Tất cả phim"), QString());
    screeningFilter->addItem(QStringLiteral("Tất cả suất"), QString());
    DoublyLinkedList<cinema::ScreeningDetails> shows;
    std::string error;
    if (!operations.screenings({}, {}, {}, std::nullopt, false, shows, error)) { errorLabel->setText(QString::fromStdString(error)); return; }
    for (const Node<cinema::ScreeningDetails>* node = shows.getHead(); node != nullptr; node = node->next)
    {
        if (filmFilter->findData(QString::fromStdString(node->data.film.filmId)) < 0)
            filmFilter->addItem(QString::fromStdString(node->data.film.title), QString::fromStdString(node->data.film.filmId));
        screeningFilter->addItem(QString::fromStdString(node->data.screening.screeningId), QString::fromStdString(node->data.screening.screeningId));
    }
    if (filmFilter->findData(previousFilm) >= 0) filmFilter->setCurrentIndex(filmFilter->findData(previousFilm));
    if (screeningFilter->findData(previousShow) >= 0) screeningFilter->setCurrentIndex(screeningFilter->findData(previousShow));
    run();
}

void ReportPage::run()
{
    clear();
    const cinema::RevenueQuery query{from->date().toString(QStringLiteral("yyyy-MM-dd")).toStdString() + " 00:00:00",
        to->date().toString(QStringLiteral("yyyy-MM-dd")).toStdString() + " 23:59:59",
        filmFilter->currentData().toString().toStdString(), screeningFilter->currentData().toString().toStdString()};
    cinema::RevenueReport report;
    std::string error;
    if (!operations.revenue(query, report, error)) { errorLabel->setText(QString::fromStdString(error)); return; }
    summary->setText(QStringLiteral("%1 vé · Doanh thu: %2 VND · Chỉ giao dịch đã xác nhận tiền mặt.").arg(report.ticketCount).arg(report.revenue));
    table->setRowCount(report.rows.size());
    int row = 0;
    for (const Node<cinema::RevenueRow>* node = report.rows.getHead(); node != nullptr; node = node->next, ++row)
        rowValues(table, row, {QString::fromStdString(node->data.filmId), QString::fromStdString(node->data.title),
                              QString::number(node->data.ticketCount), QString::number(node->data.revenue)});
    DoublyLinkedList<cinema::ScreeningDetails> shows;
    if (!operations.screenings(query.filmId, {}, {}, std::nullopt, false, shows, error))
    { errorLabel->setText(QString::fromStdString(error)); return; }
    for (const Node<cinema::ScreeningDetails>* node = shows.getHead(); node != nullptr; node = node->next)
    {
        if (!query.screeningId.empty() && node->data.screening.screeningId != query.screeningId) continue;
        cinema::OccupancyReport filled;
        if (!operations.occupancy(node->data.screening.screeningId, filled, error))
        { errorLabel->setText(QString::fromStdString(error)); return; }
        const int next = occupancy->rowCount();
        occupancy->insertRow(next);
        rowValues(occupancy, next, {QString::fromStdString(node->data.screening.screeningId), QString::fromStdString(node->data.film.title),
            QString::fromStdString(node->data.auditorium.name), QString::number(filled.soldSeats),
            QString::number(node->data.auditorium.capacity), QString::number(filled.percent, 'f', 2)});
    }
}
