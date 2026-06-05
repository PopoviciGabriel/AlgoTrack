#ifndef ALGOTRACK_MAIN_WINDOW_H
#define ALGOTRACK_MAIN_WINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <variant>
#include <memory>

// Tipuri puternice pentru mesajele IPC
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

// Utilitar C++17 pentru std::visit
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// Forward declaration pentru structura care ascunde interfața vizuală (Pimpl)
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
    void applySearch();
    void sortByDifficulty();
    void sortByTime();
    void sortByRating();
    void loadFile();
    void saveFile();
    void saveFileAs();
    void importFile();
    void exportFile();

private:
    std::unique_ptr<MainWindowImpl> ui; // Ascunde toate pointerele spre QTableWidget, etc.
    QProcess *cliProcess;

    bool readingList = false;
    bool readingStats = false;
    QString statsHtmlBuffer;

    void sendCommandToCli(const QString &command);
    void processMessage(const IPC::Message &msg);
    void addCsvRowToTable(const QString &csv);
    void populateTableSingleRow(const QString &csv);
    void processStatLine(const QString &line);
};

#endif // ALGOTRACK_MAIN_WINDOW_H