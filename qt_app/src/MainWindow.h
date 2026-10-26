#ifndef ALGOTRACK_MAIN_WINDOW_H
#define ALGOTRACK_MAIN_WINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QMap>
#include <variant>
#include <memory>

class QTimer;

namespace IPC
{
    struct Ready
    {
    };
    struct FoundExact
    {
        QString name;
        QString csv;
    };
    struct FoundFuzzy
    {
        QString name;
        QString csv;
    };
    struct NotFound
    {
    };
    struct StartList
    {
    };
    struct EndList
    {
    };
    struct StartStats
    {
    };
    struct EndStats
    {
    };
    struct Success
    {
    };
    struct Error
    {
        QString message;
    };
    struct DataLine
    {
        QString content;
    };

    using Message = std::variant<Ready, FoundExact, FoundFuzzy, NotFound,
                                 StartList, EndList, StartStats, EndStats,
                                 Success, Error, DataLine>;

    Message parseMessage(const QString &line);
}

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct MainWindowImpl;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend struct MainWindowImpl;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleCliOutput();
    void addProblem();
    void deleteSelectedProblem();
    void editSelectedProblem();
    void showDueToday();
    void reviewSelected(int rating);
    void markSelectedReviewed();
    void openSelectedProblemUrl();
    void openSelectedSolution();
    void togglePracticeTimer();
    void resetPracticeTimer();
    void updatePracticeReadout();
    void applySearch();
    void clearSearchAndFilters();
    void sortByDifficulty();
    void sortByTime();
    void sortByRating();
    void loadFile();
    void saveFile();
    void saveFileAs();
    void importFile();
    void exportFile();
    void exportMarkdown();
    void toggleDarkMode(); // Comutatorul global

private:
    std::unique_ptr<MainWindowImpl> ui;
    QProcess *cliProcess;

    bool readingList = false;
    bool readingStats = false;
    QString statsHtmlBuffer;

    int statEasy = 0;
    int statMedium = 0;
    int statHard = 0;
    int statSolved = 0;
    int statTotal = 0;
    QMap<QString, int> statDates;

    QTimer *practiceTimer = nullptr;
    qint64 practiceElapsedMs = 0;
    qint64 practiceAnchorMs = 0;
    bool practiceRunning = false;
    bool resetTimerAfterAdd = false;
    qint64 currentPracticeMs() const;
    int practiceMinutesForDialog() const;

    bool isDarkMode = true; // Starea temei
    bool showingDue = false;

    void sendCommandToCli(const QString &command);
    void refreshLibrary();
    void processMessage(const IPC::Message &msg);
    void addCsvRowToTable(const QString &csv);
    void populateTableSingleRow(const QString &csv);
    void processStatLine(const QString &line);
    void updateThemeStyles(); // Mecanismul modern de re-randare
};

#endif // ALGOTRACK_MAIN_WINDOW_H