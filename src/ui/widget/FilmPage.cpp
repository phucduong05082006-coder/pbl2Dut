#include "FilmPage.h"

#include <limits>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVariant>
#include <QVBoxLayout>

FilmPage::FilmPage(cinema::FilmController& controller, cinema::AuthController& authentication, QWidget* parent)
    : QWidget(parent), films(controller), auth(authentication)
{
    setObjectName(QStringLiteral("filmPage"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    auto* filters = new QFormLayout;
    filters->setRowWrapPolicy(QFormLayout::WrapAllRows);
    filters->setSpacing(10);
    searchInput = new QLineEdit(this);
    searchInput->setObjectName(QStringLiteral("filmSearch"));
    searchInput->setAccessibleName(QStringLiteral("Tìm phim theo mã hoặc tên"));
    searchInput->setPlaceholderText(QStringLiteral("Nhập mã hoặc tên phim; nhấn Enter để tìm"));
    filters->addRow(QStringLiteral("&Tìm phim"), searchInput);
    statusFilter = new QComboBox(this);
    statusFilter->setObjectName(QStringLiteral("filmFilter"));
    statusFilter->setAccessibleName(QStringLiteral("Lọc trạng thái phim"));
    statusFilter->addItem(QStringLiteral("Tất cả trạng thái"));
    statusFilter->addItem(QStringLiteral("Đang khai thác"));
    statusFilter->addItem(QStringLiteral("Ngừng khai thác"));
    filters->addRow(QStringLiteral("&Trạng thái"), statusFilter);
    layout->addLayout(filters);
    auto* toolbar = new QGridLayout;
    toolbar->setSpacing(12);
    addButton = new QPushButton(QStringLiteral("Thêm phim"), this);
    addButton->setObjectName(QStringLiteral("filmAdd"));
    addButton->setProperty("primary", QVariant(true));
    editButton = new QPushButton(QStringLiteral("Chỉnh sửa"), this);
    editButton->setObjectName(QStringLiteral("filmEdit"));
    detailsButton = new QPushButton(QStringLiteral("Chi tiết"), this);
    detailsButton->setObjectName(QStringLiteral("filmDetails"));
    deactivateButton = new QPushButton(QStringLiteral("Ngừng khai thác"), this);
    deactivateButton->setObjectName(QStringLiteral("filmDeactivate"));
    auto* refresh = new QPushButton(QStringLiteral("Tải lại / Tìm"), this);
    refresh->setObjectName(QStringLiteral("filmRefresh"));
    toolbar->addWidget(addButton, 0, 0);
    toolbar->addWidget(editButton, 0, 1);
    toolbar->addWidget(detailsButton, 0, 2);
    toolbar->addWidget(deactivateButton, 1, 0, 1, 2);
    toolbar->addWidget(refresh, 1, 2);
    layout->addLayout(toolbar);
    summary = new QLabel(this);
    summary->setObjectName(QStringLiteral("filmSummary"));
    layout->addWidget(summary);
    errorLabel = new QLabel(this);
    errorLabel->setObjectName(QStringLiteral("filmError"));
    errorLabel->setTextFormat(Qt::PlainText);
    errorLabel->setWordWrap(true);
    errorLabel->setAccessibleName(QStringLiteral("Lỗi thao tác phim"));
    errorLabel->hide();
    layout->addWidget(errorLabel);
    table = new QTableWidget(0, 5, this);
    table->setObjectName(QStringLiteral("filmTable"));
    table->setAccessibleName(QStringLiteral("Danh sách phim; chọn một hàng để xem chi tiết"));
    table->setHorizontalHeaderLabels({QStringLiteral("Mã phim"), QStringLiteral("Tên phim"),
        QStringLiteral("Phút"), QStringLiteral("Độ tuổi"), QStringLiteral("Trạng thái")});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setMinimumHeight(240);
    table->verticalHeader()->hide();
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setMinimumSectionSize(70);
    table->verticalHeader()->setDefaultSectionSize(44);
    layout->addWidget(table, 1);
    connect(searchInput, &QLineEdit::returnPressed, this, [this] { reload(); });
    connect(statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { reload(); });
    connect(refresh, &QPushButton::clicked, this, [this] { reload(); });
    connect(addButton, &QPushButton::clicked, this, [this] { showForm(true); });
    connect(editButton, &QPushButton::clicked, this, [this] { showForm(false); });
    connect(detailsButton, &QPushButton::clicked, this, [this] { showForm(false, true); });
    connect(deactivateButton, &QPushButton::clicked, this, [this] { deactivate(); });
    connect(table, &QTableWidget::itemSelectionChanged, this, [this] { updateActions(); });
    connect(table, &QTableWidget::cellDoubleClicked, this, [this] { showForm(false, true); });
    reload();
}

void FilmPage::refreshPermissions()
{
    cinema::LoginSession session;
    std::string error;
    const bool loggedIn = auth.currentSession(session, error);
    canManage = loggedIn && session.role == cinema::UserRole::Manager;
    addButton->setVisible(canManage);
    editButton->setVisible(canManage);
    deactivateButton->setVisible(canManage);
    if (!loggedIn)
    {
        table->setRowCount(0);
        summary->clear();
        errorLabel->setText(QString::fromStdString(error));
        errorLabel->show();
    }
    updateActions();
}

void FilmPage::updateActions()
{
    const int row = table->currentRow();
    const bool selected = row >= 0 && table->item(row, 4) != nullptr;
    detailsButton->setEnabled(selected);
    editButton->setEnabled(canManage && selected);
    deactivateButton->setEnabled(canManage && selected &&
        table->item(row, 4)->data(Qt::UserRole).toInt() == static_cast<int>(cinema::RecordStatus::Active));
}

void FilmPage::reload()
{
    refreshPermissions();
    errorLabel->clear();
    errorLabel->hide();
    std::optional<cinema::RecordStatus> status;
    if (statusFilter->currentIndex() == 1) status = cinema::RecordStatus::Active;
    if (statusFilter->currentIndex() == 2) status = cinema::RecordStatus::Inactive;
    DoublyLinkedList<cinema::Film> records;
    std::string error;
    const bool success = films.list(searchInput->text().trimmed().toStdString(), status, records, error);
    table->setRowCount(0);
    if (!success)
    {
        summary->clear();
        errorLabel->setText(QString::fromStdString(error));
        errorLabel->show();
        updateActions();
        return;
    }
    table->setRowCount(records.size());
    int row = 0;
    for (const Node<cinema::Film>* node = records.getHead(); node != nullptr; node = node->next, ++row)
    {
        const auto& film = node->data;
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(film.filmId)));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(film.title)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(film.durationMinutes)));
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(film.ageRating)));
        auto* state = new QTableWidgetItem(film.status == cinema::RecordStatus::Active
            ? QStringLiteral("Đang khai thác") : QStringLiteral("Ngừng khai thác"));
        state->setData(Qt::UserRole, static_cast<int>(film.status));
        table->setItem(row, 4, state);
    }
    summary->setText(records.isEmpty() ? QStringLiteral("Không có phim phù hợp.")
                                      : QStringLiteral("%1 phim phù hợp").arg(records.size()));
    updateActions();
}

bool FilmPage::selectedFilm(cinema::Film& film)
{
    const int row = table->currentRow();
    if (row < 0 || table->item(row, 0) == nullptr) return false;
    std::string error;
    if (films.find(table->item(row, 0)->text().toStdString(), film, error)) return true;
    errorLabel->setText(QString::fromStdString(error));
    errorLabel->show();
    return false;
}

void FilmPage::showForm(bool creating, bool readOnly)
{
    refreshPermissions();
    if (!readOnly && !canManage) return;
    cinema::Film film;
    if (!creating && !selectedFilm(film)) return;
    QDialog dialog(this);
    dialog.setObjectName(QStringLiteral("filmForm"));
    dialog.setWindowTitle(readOnly ? QStringLiteral("Chi tiết phim")
        : creating ? QStringLiteral("Thêm phim") : QStringLiteral("Chỉnh sửa phim"));
    dialog.setMinimumWidth(360);
    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
    auto* form = new QFormLayout;
    form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    form->setSpacing(10);
    auto* id = new QLineEdit(QString::fromStdString(film.filmId), &dialog);
    id->setObjectName(QStringLiteral("filmId"));
    id->setReadOnly(!creating || readOnly);
    form->addRow(QStringLiteral("&Mã phim"), id);
    auto* title = new QLineEdit(QString::fromStdString(film.title), &dialog);
    title->setObjectName(QStringLiteral("filmTitle"));
    title->setReadOnly(readOnly);
    form->addRow(QStringLiteral("&Tên phim"), title);
    auto* duration = new QSpinBox(&dialog);
    duration->setObjectName(QStringLiteral("filmDuration"));
    duration->setRange(1, std::numeric_limits<int>::max());
    duration->setValue(creating ? 90 : film.durationMinutes);
    duration->setSuffix(QStringLiteral(" phút"));
    duration->setReadOnly(readOnly);
    form->addRow(QStringLiteral("Thời &lượng"), duration);
    auto* rating = new QLineEdit(QString::fromStdString(film.ageRating), &dialog);
    rating->setObjectName(QStringLiteral("filmRating"));
    rating->setReadOnly(readOnly);
    form->addRow(QStringLiteral("Phân loại độ &tuổi"), rating);
    auto* status = new QComboBox(&dialog);
    status->setObjectName(QStringLiteral("filmStatus"));
    status->addItem(QStringLiteral("Đang khai thác"));
    status->addItem(QStringLiteral("Ngừng khai thác"));
    status->setCurrentIndex(film.status == cinema::RecordStatus::Active ? 0 : 1);
    status->setEnabled(!readOnly);
    form->addRow(QStringLiteral("Trạng thá&i"), status);
    layout->addLayout(form);
    auto* validation = new QLabel(&dialog);
    validation->setObjectName(QStringLiteral("filmFormError"));
    validation->setTextFormat(Qt::PlainText);
    validation->setWordWrap(true);
    validation->setFocusPolicy(Qt::StrongFocus);
    validation->setAccessibleName(QStringLiteral("Lỗi lưu phim"));
    validation->hide();
    layout->addWidget(validation);
    auto* buttons = new QDialogButtonBox(readOnly ? QDialogButtonBox::Close
        : QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    if (!readOnly)
    {
        buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("Lưu"));
        buttons->button(QDialogButtonBox::Save)->setObjectName(QStringLiteral("filmSave"));
        buttons->button(QDialogButtonBox::Save)->setProperty("primary", QVariant(true));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Hủy"));
    }
    else buttons->button(QDialogButtonBox::Close)->setText(QStringLiteral("Đóng"));
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        const cinema::Film request{id->text().trimmed().toStdString(), title->text().trimmed().toStdString(),
            duration->value(), rating->text().trimmed().toStdString(),
            status->currentIndex() == 0 ? cinema::RecordStatus::Active : cinema::RecordStatus::Inactive};
        std::string error;
        if (creating ? films.create(request, error) : films.update(request, error)) dialog.accept();
        else { validation->setText(QString::fromStdString(error)); validation->show(); validation->setFocus(); }
    });
    if (creating) id->setFocus();
    else title->setFocus();
    if (dialog.exec() == QDialog::Accepted) reload();
}

void FilmPage::deactivate()
{
    refreshPermissions();
    if (!canManage) return;
    cinema::Film film;
    if (!selectedFilm(film)) return;
    if (QMessageBox::question(this, QStringLiteral("Ngừng khai thác phim"),
        QStringLiteral("Ngừng khai thác phim có mã %1?").arg(QString::fromStdString(film.filmId)),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
    film.status = cinema::RecordStatus::Inactive;
    std::string error;
    if (films.update(film, error)) reload();
    else { errorLabel->setText(QString::fromStdString(error)); errorLabel->show(); }
}
