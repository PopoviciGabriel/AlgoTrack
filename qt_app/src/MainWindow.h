#pragma once

#include "ProblemManager.h"

#include <QMainWindow>
#include <QString>
#include <vector>

class QLabel;
class QLineEdit;
class QTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    ProblemManager manager;
    QString currentFile;

    QTableWidget* table;
    QLineEdit* searchEdit;
    QLabel* statusLabel;
    QLabel* statsLabel;
    QLabel* totalValue;
    QLabel* solvedValue;
    QLabel* ratingValue;
    QLabel* timeValue;

    void buildUi();
    void applyTheme();
    void refreshTable(const std::vector<Problem>& problems);
    void refreshAll();
    void refreshStats();
    void loadDefaultFile();
    void autoSave();
    QString defaultDataPath() const;
    void loadFromPath(const QString& path);
    bool saveToPath(const QString& path);
    int selectedSourceIndex() const;

    static QString tagsToString(const std::vector<std::string>& tags);
    static QString problemToSearchText(const Problem& problem);

private slots:
    void addProblem();
    void deleteSelectedProblem();
    void loadFile();
    void saveFile();
    void saveFileAs();
    void importFile();
    void exportFile();
    void applySearch();
    void sortByDifficulty();
    void sortByTime();
    void sortByRating();
};

