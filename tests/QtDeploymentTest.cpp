#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QProcess>
#include <QTemporaryDir>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>

#include "service/AuthService.h"
#include "service/ReportService.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    assert(argc == 2);
    QTemporaryDir temporary;
    assert(temporary.isValid());
    const QString path = temporary.filePath(QStringLiteral("demo"));
    const auto run = [&](const QString& target, bool success) {
        QProcess process;
        process.start(QString::fromLocal8Bit(argv[1]), {QStringLiteral("--prepare-demo"), QStringLiteral("--data-dir"), target});
        assert(process.waitForFinished(20000));
        assert(process.exitStatus() == QProcess::NormalExit);
        assert((process.exitCode() == 0) == success);
    };
    run(path, true);
    cinema::ProjectDataEngine engine(path.toStdString());
    std::string error;
    assert(engine.open(error));
    assert(engine.query<cinema::Film>().size() == 3);
    assert(engine.query<cinema::Auditorium>().size() == 3);
    assert(engine.query<cinema::Seat>().size() == 136);
    assert(engine.query<cinema::Screening>().size() == 6);
    assert(engine.query<cinema::Ticket>().size() == 6);
    assert(engine.query<cinema::SaleTransaction>().size() == 3);
    cinema::AuthService auth(engine);
    cinema::LoginSession session;
    assert(auth.login("manager", "demo123", session, error));
    assert(!auth.initializeManager("other", "Other", "secret", error));
    cinema::ReportService reports(engine, auth);
    cinema::RevenueReport revenue;
    assert(reports.revenueByFilm({}, revenue, error));
    assert(revenue.revenue == 480000 && revenue.ticketCount == 6);
    const auto read = [&] {
        std::ifstream input((path + QStringLiteral("/users.txt")).toStdString());
        return std::string(std::istreambuf_iterator<char>(input), {});
    };
    const auto original = read();
    run(path, false);
    assert(read() == original);
    QLockFile lock(path + QStringLiteral("/.cinema.lock"));
    assert(lock.tryLock(0));
    const auto alias = temporary.filePath(QStringLiteral("alias"));
    std::filesystem::create_directory_symlink(path.toStdString(), alias.toStdString());
    run(alias, false);
    assert(read() == original);
}
