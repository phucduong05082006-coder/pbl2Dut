#include "MainWindow.h"

#include <QFrame>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QTimer>
#include <QVariant>
#include <QVBoxLayout>

#include "../widget/AccountPage.h"
#include "../widget/AuditoriumPage.h"
#include "../widget/FilmPage.h"
#include "../widget/OperationsPages.h"

MainWindow::MainWindow(cinema::AuthController& authentication, cinema::FilmController& films,
                       cinema::AuditoriumController& auditoriums, cinema::OperationsController& operations,
                       const QString& dataPath, QWidget* parent)
    : QMainWindow(parent), auth(authentication)
{
    setWindowTitle(QStringLiteral("FRAME | Quản lý rạp chiếu phim"));
    setMinimumSize(800, 560);
    resize(1200, 800);
    auto* shell = new QWidget(this);
    auto* root = new QHBoxLayout(shell);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    auto* sidebar = new QFrame(shell);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    auto* navigation = new QVBoxLayout(sidebar);
    navigation->setContentsMargins(18, 28, 18, 20);
    navigation->setSpacing(10);
    auto* brand = new QLabel(QStringLiteral("FRAME."), sidebar);
    brand->setObjectName(QStringLiteral("brand"));
    navigation->addWidget(brand);
    auto* caption = new QLabel(QStringLiteral("CINEMA OPERATIONS"), sidebar);
    caption->setObjectName(QStringLiteral("muted"));
    navigation->addWidget(caption);
    navigation->addSpacing(22);
    auto* overview = new QLabel(QStringLiteral("VẬN HÀNH RẠP"), sidebar);
    overview->setObjectName(QStringLiteral("eyebrow"));
    navigation->addWidget(overview);
    overviewNavigation = new QPushButton(QStringLiteral("Tổng quan"), sidebar);
    overviewNavigation->setObjectName(QStringLiteral("navOverview"));
    navigation->addWidget(overviewNavigation);
    filmsNavigation = new QPushButton(QStringLiteral("Phim"), sidebar);
    filmsNavigation->setObjectName(QStringLiteral("navFilms"));
    filmsNavigation->setProperty("nav", QVariant(true));
    filmsNavigation->setCheckable(true);
    filmsNavigation->setChecked(true);
    navigation->addWidget(filmsNavigation);
    auditoriumsNavigation = new QPushButton(QStringLiteral("Phòng && ghế"), sidebar);
    auditoriumsNavigation->setObjectName(QStringLiteral("navAuditoriums"));
    auditoriumsNavigation->setProperty("nav", QVariant(true));
    auditoriumsNavigation->setCheckable(true);
    navigation->addWidget(auditoriumsNavigation);
    screeningsNavigation = new QPushButton(QStringLiteral("Lịch chiếu && giá"), sidebar);
    screeningsNavigation->setObjectName(QStringLiteral("navScreenings"));
    salesNavigation = new QPushButton(QStringLiteral("Tra suất / Bán vé"), sidebar);
    salesNavigation->setObjectName(QStringLiteral("navSales"));
    lookupNavigation = new QPushButton(QStringLiteral("Tra cứu vé / giao dịch"), sidebar);
    lookupNavigation->setObjectName(QStringLiteral("navLookup"));
    reportsNavigation = new QPushButton(QStringLiteral("Báo cáo"), sidebar);
    reportsNavigation->setObjectName(QStringLiteral("navReports"));
    for (auto* button : {screeningsNavigation, salesNavigation, lookupNavigation, reportsNavigation})
        navigation->addWidget(button);
    accountsNavigation = new QPushButton(QStringLiteral("Tài khoản && quyền"), sidebar);
    accountsNavigation->setObjectName(QStringLiteral("navAccounts"));
    accountsNavigation->setProperty("nav", QVariant(true));
    accountsNavigation->setCheckable(true);
    navigation->addWidget(accountsNavigation);
    auto* buttonGroup = new QButtonGroup(this);
    for (auto* button : {overviewNavigation, filmsNavigation, auditoriumsNavigation, screeningsNavigation,
                         salesNavigation, lookupNavigation, reportsNavigation, accountsNavigation})
    {
        button->setProperty("nav", QVariant(true));
        button->setCheckable(true);
        buttonGroup->addButton(button);
    }
    navigation->addStretch();
    auto* offline = new QLabel(QStringLiteral("OFFLINE\nDữ liệu lưu trên máy này"), sidebar);
    offline->setObjectName(QStringLiteral("muted"));
    navigation->addWidget(offline);
    auto* logout = new QPushButton(QStringLiteral("Đăng xuất"), sidebar);
    logout->setObjectName(QStringLiteral("logout"));
    navigation->addWidget(logout);
    connect(logout, &QPushButton::clicked, this, [this] { auth.logout(); close(); });
    auto* navigationScroll = new QScrollArea(shell);
    navigationScroll->setObjectName(QStringLiteral("navigationScroll"));
    navigationScroll->setFixedWidth(208);
    navigationScroll->setFrameShape(QFrame::NoFrame);
    navigationScroll->setWidgetResizable(true);
    navigationScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigationScroll->setWidget(sidebar);
    root->addWidget(navigationScroll);

    auto* workspace = new QWidget(shell);
    auto* body = new QVBoxLayout(workspace);
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);
    auto* topbar = new QFrame(workspace);
    topbar->setObjectName(QStringLiteral("topbar"));
    auto* header = new QVBoxLayout(topbar);
    header->setContentsMargins(24, 14, 24, 14);
    header->setSpacing(8);
    identity = new QLabel(topbar);
    identity->setObjectName(QStringLiteral("identity"));
    identity->setTextFormat(Qt::PlainText);
    identity->setWordWrap(true);
    permissions = new QLabel(topbar);
    permissions->setObjectName(QStringLiteral("permissions"));
    permissions->setTextFormat(Qt::PlainText);
    permissions->setWordWrap(false);
    permissions->setMinimumWidth(360);
    permissions->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    header->addWidget(identity);
    header->addWidget(permissions, 0, Qt::AlignLeft);
    if (!dataPath.isEmpty())
    {
        auto* location = new QLabel(QStringLiteral("Dữ liệu TXT: %1").arg(dataPath), topbar);
        location->setObjectName(QStringLiteral("dataLocation"));
        location->setTextFormat(Qt::PlainText);
        location->setWordWrap(true);
        location->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        header->addWidget(location);
    }
    body->addWidget(topbar);

    auto* pageHeader = new QWidget(workspace);
    auto* pageHeading = new QVBoxLayout(pageHeader);
    pageHeading->setContentsMargins(24, 22, 24, 8);
    pageHeading->setSpacing(8);
    pageTitle = new QLabel(QStringLiteral("Quản lý phim"), pageHeader);
    pageTitle->setObjectName(QStringLiteral("pageTitle"));
    pageTitle->setWordWrap(true);
    pageDescription = new QLabel(QStringLiteral("Danh mục phim được khai thác tại rạp."), pageHeader);
    pageDescription->setObjectName(QStringLiteral("pageDescription"));
    pageDescription->setWordWrap(true);
    pageHeading->addWidget(pageTitle);
    pageHeading->addWidget(pageDescription);
    body->addWidget(pageHeader);
    auto* scroll = new QScrollArea(workspace);
    scroll->setWidgetResizable(true);
    scroll->setObjectName(QStringLiteral("workspaceScroll"));
    pages = new QStackedWidget;
    pages->setObjectName(QStringLiteral("workspacePages"));
    filmPage = new FilmPage(films, auth, pages);
    accountPage = new AccountPage(auth, pages);
    auditoriumPage = new AuditoriumPage(auditoriums, auth, pages);
    pages->addWidget(filmPage);
    pages->addWidget(accountPage);
    pages->addWidget(auditoriumPage);
    screeningPage = new ScreeningPage(operations, films, auditoriums, auth, pages);
    salePage = new SalePage(operations, films, auditoriums, auth, pages);
    lookupPage = new LookupPage(operations, pages);
    reportPage = new ReportPage(operations, pages);
    overviewPage = new QWidget(pages);
    auto* overviewLayout = new QVBoxLayout(overviewPage);
    overviewLayout->setContentsMargins(24, 16, 24, 24);
    auto* overviewText = new QLabel(QStringLiteral("Vận hành một rạp, nhiều phòng, dữ liệu TXT cục bộ.\n\n"
        "1. Khai báo phim và phòng.\n2. Tạo suất, kiểm tra lịch và mở bán.\n"
        "3. Chọn suất, chọn ghế, kiểm tra tiền và xác nhận nhận tiền mặt.\n"
        "4. Tra cứu vé/giao dịch và xem báo cáo.\n\n"
        "Quyền bán vé được cấp riêng; Admin không mặc nhiên được bán."), overviewPage);
    overviewText->setWordWrap(true);
    overviewLayout->addWidget(overviewText);
    for (auto* target : {filmsNavigation, auditoriumsNavigation, screeningsNavigation, salesNavigation, lookupNavigation, reportsNavigation})
    {
        auto* shortcut = new QPushButton(target->text(), overviewPage);
        overviewLayout->addWidget(shortcut);
        connect(shortcut, &QPushButton::clicked, target, &QPushButton::click);
    }
    overviewLayout->addStretch();
    pages->addWidget(screeningPage);
    pages->addWidget(salePage);
    pages->addWidget(lookupPage);
    pages->addWidget(reportPage);
    pages->addWidget(overviewPage);
    scroll->setWidget(pages);
    body->addWidget(scroll, 1);
    root->addWidget(workspace, 1);
    setCentralWidget(shell);
    connect(filmsNavigation, &QPushButton::clicked, this, [this] {
        pages->setCurrentWidget(filmPage);
        filmsNavigation->setChecked(true);
        accountsNavigation->setChecked(false);
        auditoriumsNavigation->setChecked(false);
        pageTitle->setText(QStringLiteral("Quản lý phim"));
        pageDescription->setText(QStringLiteral("Danh mục phim được khai thác tại rạp."));
    });
    connect(auditoriumsNavigation, &QPushButton::clicked, this, [this] {
        auditoriumPage->reload();
        pages->setCurrentWidget(auditoriumPage);
        auditoriumsNavigation->setChecked(true);
        filmsNavigation->setChecked(false);
        accountsNavigation->setChecked(false);
        pageTitle->setText(QStringLiteral("Phòng chiếu & sơ đồ ghế"));
        pageDescription->setText(QStringLiteral("Chọn phòng để xem các ghế thực tế trong bộ dữ liệu đang mở."));
    });
    connect(accountsNavigation, &QPushButton::clicked, this, [this] {
        refreshSession();
        if (!accountsNavigation->isVisible()) return;
        accountPage->reload();
        pages->setCurrentWidget(accountPage);
        filmsNavigation->setChecked(false);
        accountsNavigation->setChecked(true);
        auditoriumsNavigation->setChecked(false);
        pageTitle->setText(QStringLiteral("Tài khoản & quyền"));
        pageDescription->setText(QStringLiteral("Admin và nhân viên. Quyền quản lý và quyền bán vé được kiểm soát riêng."));
    });
    connect(overviewNavigation, &QPushButton::clicked, this, [this] {
        cinema::LoginSession session;
        std::string error;
        if (!auth.currentSession(session, error) || session.role != cinema::UserRole::Manager) return;
        pages->setCurrentWidget(overviewPage);
        pageTitle->setText(QStringLiteral("Tổng quan vận hành"));
        pageDescription->setText(QStringLiteral("Lối vào các nghiệp vụ quản lý rạp và quầy vé."));
    });
    connect(screeningsNavigation, &QPushButton::clicked, this, [this] {
        refreshSession();
        if (!screeningsNavigation->isVisible()) return;
        screeningPage->reload();
        pages->setCurrentWidget(screeningPage);
        pageTitle->setText(QStringLiteral("Lịch chiếu & giá vé"));
        pageDescription->setText(QStringLiteral("Tạo lịch, công bố hoặc đóng bán. Luôn tính 15 phút nghỉ phòng."));
    });
    connect(salesNavigation, &QPushButton::clicked, this, [this] {
        salePage->reload();
        pages->setCurrentWidget(salePage);
        pageTitle->setText(QStringLiteral("Tra cứu suất & bán vé tại quầy"));
        pageDescription->setText(QStringLiteral("Ghế theo từng suất. Chỉ phát hành vé sau khi xác nhận đủ tiền mặt."));
    });
    connect(lookupNavigation, &QPushButton::clicked, this, [this] {
        lookupPage->clear();
        pages->setCurrentWidget(lookupPage);
        pageTitle->setText(QStringLiteral("Tra cứu vé & giao dịch"));
        pageDescription->setText(QStringLiteral("Đối chiếu phim, suất, phòng, ghế, giá và người thực hiện."));
    });
    connect(reportsNavigation, &QPushButton::clicked, this, [this] {
        refreshSession();
        if (!reportsNavigation->isVisible()) return;
        reportPage->reload();
        pages->setCurrentWidget(reportPage);
        pageTitle->setText(QStringLiteral("Báo cáo bán vé"));
        pageDescription->setText(QStringLiteral("Doanh thu theo ngày bán. Tỷ lệ lấp đầy = vé đã bán / sức chứa phòng."));
    });
    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this] { refreshSession(); });
    timer->start(5000);
    refreshSession();
    cinema::LoginSession session;
    std::string error;
    if (auth.currentSession(session, error))
    {
        if (session.role == cinema::UserRole::Manager) overviewNavigation->click();
        else salesNavigation->click();
    }
}

void MainWindow::refreshSession()
{
    filmPage->refreshPermissions();
    accountPage->refreshPermissions();
    auditoriumPage->refreshPermissions();
    screeningPage->refreshPermissions();
    salePage->refreshPermissions();
    cinema::LoginSession session;
    std::string error;
    if (!auth.currentSession(session, error))
    {
        identity->clear();
        permissions->clear();
        accountsNavigation->hide();
        lookupPage->clear();
        reportPage->clear();
        auth.logout();
        QTimer::singleShot(0, this, [this] { close(); });
        return;
    }
    const bool manager = session.role == cinema::UserRole::Manager;
    accountsNavigation->setVisible(manager);
    overviewNavigation->setVisible(manager);
    screeningsNavigation->setVisible(manager);
    reportsNavigation->setVisible(manager);
    if (!manager)
    {
        reportPage->clear();
        if (pages->currentWidget() == accountPage || pages->currentWidget() == overviewPage ||
            pages->currentWidget() == screeningPage || pages->currentWidget() == reportPage) salesNavigation->click();
    }
    identity->setText(QStringLiteral("%1  ·  @%2")
        .arg(QString::fromStdString(session.displayName), QString::fromStdString(session.username)));
    permissions->setText(QStringLiteral("%1  ·  Bán vé: %2")
        .arg(manager ? QStringLiteral("Admin (Quản lý)") : QStringLiteral("Nhân viên"),
             session.canSellTicket ? QStringLiteral("Được cấp") : QStringLiteral("Chưa cấp")));
}
