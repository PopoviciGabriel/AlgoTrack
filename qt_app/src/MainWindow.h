#ifndef ALGOTRACK_MAIN_WINDOW_H
#define ALGOTRACK_MAIN_WINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QString>
#include <QStringList>

// Forward Declarations pentru widget-uri
class QTableWidget;
class QLineEdit;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

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
    QTableWidget *table;
    QLineEdit *searchEdit;
    QLabel *statusLabel;
    QLabel *statsLabel;
    QLabel *totalValue;
    QLabel *solvedValue;
    QLabel *ratingValue;
    QLabel *timeValue;

    // Proprietăți specifice controlului IPC (Pasul 1)
    QProcess *cliProcess;
    QString outputBuffer;

    void buildUi();
    void applyTheme();
    void sendCommandToCli(const QString &command);
    void parseIncomingLine(const QString &line);
};

#endif // ALGOTRACK_MAIN_WINDOW_H
