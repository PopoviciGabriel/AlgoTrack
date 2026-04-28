#include "MainWindow.h"

#include "AddProblemDialog.h"
#include "ProblemRepository.h"
#include "Statistics.h"
#include "Utils.h"

#include <QAction>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      table(new QTableWidget(this)),
      searchEdit(new QLineEdit(this)),
      statusLabel(new QLabel(this)),
      statsLabel(new QLabel(this)),
      totalValue(new QLabel("0", this)),
      solvedValue(new QLabel("0", this)),
      ratingValue(new QLabel("0.0", this)),
      timeValue(new QLabel("0m", this)) {
    buildUi();
    applyTheme();
    loadDefaultFile();
    refreshAll();
    autoSave();
}

static QPushButton* makeCommandButton(const QString& text, QWidget* parent) {
    auto* button = new QPushButton(text, parent);
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(38);
    return button;
}

static QFrame* makeMetricCard(const QString& label, QLabel* value, QWidget* parent) {
    auto* card = new QFrame(parent);
    card->setObjectName("metricCard");

    auto* labelWidget = new QLabel(label, card);
    labelWidget->setObjectName("metricLabel");
    value->setObjectName("metricValue");

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(4);
    layout->addWidget(labelWidget);
    layout->addWidget(value);

    return card;
}

void MainWindow::buildUi() {
    setWindowTitle("AlgoTrack");
    resize(1180, 700);
    setMinimumSize(780, 520);

    auto* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("Open CSV", this, &MainWindow::loadFile);
    fileMenu->addAction("Save", this, &MainWindow::saveFile);
    fileMenu->addAction("Save As", this, &MainWindow::saveFileAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Import CSV", this, &MainWindow::importFile);
    fileMenu->addAction("Export CSV", this, &MainWindow::exportFile);

    auto* sortMenu = menuBar()->addMenu("&Sort");
    sortMenu->addAction("By difficulty", this, &MainWindow::sortByDifficulty);
    sortMenu->addAction("By time", this, &MainWindow::sortByTime);
    sortMenu->addAction("By rating", this, &MainWindow::sortByRating);

    auto* toolbar = addToolBar("Quick actions");
    toolbar->setMovable(false);
    toolbar->addAction("Add", this, &MainWindow::addProblem);
    toolbar->addAction("Delete", this, &MainWindow::deleteSelectedProblem);
    toolbar->addSeparator();
    toolbar->addAction("Open", this, &MainWindow::loadFile);
    toolbar->addAction("Save", this, &MainWindow::saveFile);

    auto* root = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(18, 16, 18, 12);
    rootLayout->setSpacing(12);

    auto* topRow = new QHBoxLayout;

    auto* titleBox = new QVBoxLayout;
    auto* title = new QLabel("AlgoTrack", this);
    title->setObjectName("appTitle");
    auto* subtitle = new QLabel("Problems, progress, and patterns in one calm workspace.", this);
    subtitle->setObjectName("appSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);

    auto* addButton = makeCommandButton("Add problem", this);
    addButton->setObjectName("primaryButton");
    auto* openButton = makeCommandButton("Open CSV", this);
    auto* saveButton = makeCommandButton("Save", this);

    topRow->addLayout(titleBox, 1);
    topRow->addWidget(openButton);
    topRow->addWidget(saveButton);
    topRow->addWidget(addButton);
    rootLayout->addLayout(topRow);

    auto* metrics = new QGridLayout;
    metrics->setHorizontalSpacing(12);
    metrics->setColumnStretch(0, 1);
    metrics->setColumnStretch(1, 1);
    metrics->setColumnStretch(2, 1);
    metrics->setColumnStretch(3, 1);
    metrics->addWidget(makeMetricCard("Total", totalValue, this), 0, 0);
    metrics->addWidget(makeMetricCard("Solved", solvedValue, this), 0, 1);
    metrics->addWidget(makeMetricCard("Avg rating", ratingValue, this), 0, 2);
    metrics->addWidget(makeMetricCard("Total time", timeValue, this), 0, 3);
    rootLayout->addLayout(metrics);

    searchEdit->setPlaceholderText("Search by name, platform, tag, status, notes...");
    searchEdit->setMinimumHeight(40);

    auto* searchButton = makeCommandButton("Search", this);
    auto* clearButton = makeCommandButton("Clear", this);
    auto* sortDifficultyButton = makeCommandButton("Difficulty", this);
    auto* sortTimeButton = makeCommandButton("Time", this);
    auto* sortRatingButton = makeCommandButton("Rating", this);

    auto* searchRow = new QHBoxLayout;
    searchRow->setSpacing(10);
    searchRow->addWidget(searchEdit, 1);
    searchRow->addWidget(searchButton);
    searchRow->addWidget(clearButton);
    searchRow->addSpacing(10);
    searchRow->addWidget(sortDifficultyButton);
    searchRow->addWidget(sortTimeButton);
    searchRow->addWidget(sortRatingButton);
    rootLayout->addLayout(searchRow);

    table->setColumnCount(9);
    table->setHorizontalHeaderLabels({
        "Name", "Platform", "Difficulty", "Tags", "Status",
        "Time", "Date", "Rating", "Notes"
    });
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setWordWrap(false);

    auto* tablePanel = new QFrame(this);
    tablePanel->setObjectName("tablePanel");
    auto* tableLayout = new QVBoxLayout(tablePanel);
    tableLayout->setContentsMargins(1, 1, 1, 1);
    tableLayout->addWidget(table);

    statsLabel->setObjectName("statsPanel");
    statsLabel->setMinimumWidth(220);
    statsLabel->setMaximumWidth(280);
    statsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    statsLabel->setWordWrap(true);

    auto* splitter = new QSplitter(this);
    auto* statsScroll = new QScrollArea(this);
    statsScroll->setObjectName("statsScroll");
    statsScroll->setWidget(statsLabel);
    statsScroll->setWidgetResizable(true);
    statsScroll->setFrameShape(QFrame::NoFrame);
    statsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    statsScroll->setMinimumWidth(220);
    statsScroll->setMaximumWidth(280);

    splitter->addWidget(tablePanel);
    splitter->addWidget(statsScroll);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setSizes({860, 240});
    rootLayout->addWidget(splitter, 1);

    setCentralWidget(root);
    statusBar()->addWidget(statusLabel, 1);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::addProblem);
    connect(openButton, &QPushButton::clicked, this, &MainWindow::loadFile);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveFile);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::applySearch);
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        searchEdit->clear();
        refreshAll();
    });
    connect(sortDifficultyButton, &QPushButton::clicked, this, &MainWindow::sortByDifficulty);
    connect(sortTimeButton, &QPushButton::clicked, this, &MainWindow::sortByTime);
    connect(sortRatingButton, &QPushButton::clicked, this, &MainWindow::sortByRating);
    connect(searchEdit, &QLineEdit::returnPressed, this, &MainWindow::applySearch);
}

void MainWindow::applyTheme() {
    setStyleSheet(R"(
        QMainWindow, QWidget {
            background: #f6f7fb;
            color: #161b26;
        }
        QMenuBar {
            background: #ffffff;
            border-bottom: 1px solid #e6e9ef;
            padding: 4px 8px;
        }
        QMenuBar::item {
            padding: 7px 10px;
            border-radius: 6px;
        }
        QMenuBar::item:selected {
            background: #eef4ff;
        }
        QMenu {
            background: white;
            border: 1px solid #d8dee9;
            padding: 6px;
        }
        QMenu::item {
            padding: 7px 22px;
            border-radius: 6px;
        }
        QMenu::item:selected {
            background: #eef4ff;
        }
        QToolBar {
            background: #ffffff;
            border: 0;
            border-bottom: 1px solid #e6e9ef;
            spacing: 6px;
            padding: 6px 10px;
        }
        QToolButton {
            border-radius: 7px;
            padding: 7px 10px;
            background: transparent;
        }
        QToolButton:hover {
            background: #eef4ff;
        }
        QLabel#appTitle {
            font-size: 22px;
            font-weight: 800;
            color: #111827;
        }
        QLabel#appSubtitle {
            color: #667085;
            font-size: 12px;
        }
        QFrame#metricCard {
            background: #ffffff;
            border: 1px solid #e3e7ee;
            border-radius: 10px;
        }
        QLabel#metricLabel {
            color: #667085;
            font-size: 12px;
        }
        QLabel#metricValue {
            color: #111827;
            font-size: 22px;
            font-weight: 750;
        }
        QFrame#tablePanel {
            background: #ffffff;
            border: 1px solid #e3e7ee;
            border-radius: 12px;
        }
        QLineEdit {
            min-height: 36px;
            border: 1px solid #d7dce5;
            border-radius: 10px;
            padding: 7px 12px;
            background: #ffffff;
            selection-background-color: #2563eb;
        }
        QLineEdit:focus {
            border: 1px solid #2563eb;
        }
        QPushButton {
            border: 1px solid #d7dce5;
            border-radius: 10px;
            padding: 8px 14px;
            background: #ffffff;
            color: #1f2937;
        }
        QPushButton:hover {
            background: #eef4ff;
            border-color: #b7c9f8;
        }
        QPushButton#primaryButton {
            background: #2563eb;
            border-color: #2563eb;
            color: #ffffff;
            font-weight: 700;
        }
        QPushButton#primaryButton:hover {
            background: #1d4ed8;
            border-color: #1d4ed8;
        }
        QTableWidget {
            border: 0;
            background: #ffffff;
            alternate-background-color: #f9fafc;
            gridline-color: #edf0f5;
            selection-background-color: #dbeafe;
            selection-color: #111827;
        }
        QTableWidget::item {
            padding: 8px;
            border: 0;
        }
        QHeaderView::section {
            background: #f1f4f9;
            color: #475467;
            border: 0;
            border-bottom: 1px solid #e3e7ee;
            padding: 9px 8px;
            font-weight: 700;
        }
        QLabel {
            color: #1f2937;
            background: transparent;
        }
        QScrollArea#statsScroll {
            background: #ffffff;
            border: 1px solid #e3e7ee;
            border-radius: 12px;
        }
        QLabel#statsPanel {
            background: #ffffff;
            border: 0;
            padding: 14px;
        }
        QStatusBar {
            background: #ffffff;
            border-top: 1px solid #e6e9ef;
            color: #667085;
        }
    )");
}

void MainWindow::refreshTable(const std::vector<Problem>& problems) {
    table->setRowCount(static_cast<int>(problems.size()));

    for (int row = 0; row < static_cast<int>(problems.size()); ++row) {
        const Problem& p = problems[row];

        auto makeItem = [](const QString& text) {
            auto* item = new QTableWidgetItem(text);
            item->setToolTip(text);
            return item;
        };

        table->setItem(row, 0, makeItem(QString::fromStdString(p.getName())));
        table->setItem(row, 1, makeItem(QString::fromStdString(p.getPlatform())));
        table->setItem(row, 2, makeItem(QString::fromStdString(difficultyToString(p.getDifficulty()))));
        table->setItem(row, 3, makeItem(tagsToString(p.getTags())));
        table->setItem(row, 4, makeItem(QString::fromStdString(statusToString(p.getStatus()))));
        table->setItem(row, 5, makeItem(QString::number(p.getTimeSpent())));
        table->setItem(row, 6, makeItem(QString::fromStdString(p.getDate())));
        table->setItem(row, 7, makeItem(QString::number(p.getRating(), 'f', 1)));
        table->setItem(row, 8, makeItem(QString::fromStdString(p.getNotes())));
    }
}

void MainWindow::refreshAll() {
    refreshTable(manager.getAllProblems());
    refreshStats();

    const QString fileText = currentFile.isEmpty() ? "No CSV loaded" : currentFile;
    statusLabel->setText(QString("%1 problems | %2")
        .arg(manager.getAllProblems().size())
        .arg(fileText));
}

void MainWindow::refreshStats() {
    const StatisticsSummary summary = calculateStatistics(manager.getAllProblems());

    totalValue->setText(QString::number(summary.totalProblems));
    solvedValue->setText(QString::number(summary.solved));
    ratingValue->setText(QString::number(summary.averageRating, 'f', 1));
    timeValue->setText(QString("%1m").arg(summary.totalTime));

    QString text;
    text += "<div style='font-size:18px; font-weight:800; margin-bottom:8px;'>Details</div>";
    text += "<div style='font-size:12px; line-height:1.25;'>";
    text += QString("<div style='font-weight:750; margin-top:4px; margin-bottom:3px;'>Status</div>"
                    "<div>Solved: %1</div><div>Failed: %2</div><div>In progress: %3</div>")
        .arg(summary.solved)
        .arg(summary.failed)
        .arg(summary.inProgress);
    text += QString("<div style='font-weight:750; margin-top:12px; margin-bottom:3px;'>Difficulty</div>"
                    "<div>Easy: %1</div><div>Medium: %2</div><div>Hard: %3</div>")
        .arg(summary.easy)
        .arg(summary.medium)
        .arg(summary.hard);
    text += QString("<div style='font-weight:750; margin-top:12px; margin-bottom:3px;'>Time</div>"
                    "<div>Average: %1 min</div><div>Longest: %2 min</div>")
        .arg(summary.averageTime, 0, 'f', 1)
        .arg(summary.longestProblemTime);
    text += QString("<div style='font-weight:750; margin-top:12px; margin-bottom:3px;'>Most used tag</div>"
                    "<div>%1</div>")
        .arg(summary.mostUsedTag.empty()
            ? "none"
            : QString("%1 (%2)").arg(QString::fromStdString(summary.mostUsedTag)).arg(summary.mostUsedTagCount));
    text += "</div>";

    statsLabel->setObjectName("statsPanel");
    statsLabel->setText(text);
}

void MainWindow::loadDefaultFile() {
    const QString path = defaultDataPath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    LoadResult result = ProblemRepository::loadFromFile(path.toStdString());
    currentFile = path;

    if (result.fileOpened) {
        manager.setProblems(result.problems);
    } else {
        ProblemRepository::saveToFile(path.toStdString(), manager.getAllProblems());
    }
}

void MainWindow::autoSave() {
    if (currentFile.isEmpty()) {
        currentFile = defaultDataPath();
    }

    QDir().mkpath(QFileInfo(currentFile).absolutePath());
    ProblemRepository::saveToFile(currentFile.toStdString(), manager.getAllProblems());
}

QString MainWindow::defaultDataPath() const {
    return QDir(QCoreApplication::applicationDirPath()).filePath("data/problems.csv");
}

void MainWindow::loadFromPath(const QString& path) {
    LoadResult result = ProblemRepository::loadFromFile(path.toStdString());
    if (!result.fileOpened) {
        QMessageBox::warning(this, "Open failed", "Could not open the selected CSV file.");
        return;
    }

    manager.setProblems(result.problems);
    currentFile = path;
    refreshAll();

    QMessageBox::information(this, "CSV loaded",
        QString("Loaded: %1\nDuplicates skipped: %2\nInvalid rows: %3")
            .arg(result.loadedCount)
            .arg(result.duplicateCount)
            .arg(result.invalidCount));
}

bool MainWindow::saveToPath(const QString& path) {
    if (!ProblemRepository::saveToFile(path.toStdString(), manager.getAllProblems())) {
        QMessageBox::warning(this, "Save failed", "Could not save the CSV file.");
        return false;
    }

    currentFile = path;
    refreshAll();
    return true;
}

int MainWindow::selectedSourceIndex() const {
    const int row = table->currentRow();
    if (row < 0 || !table->item(row, 0)) {
        return -1;
    }

    const QString name = table->item(row, 0)->text();
    const QString platform = table->item(row, 1)->text();
    const auto& problems = manager.getAllProblems();

    for (int i = 0; i < static_cast<int>(problems.size()); ++i) {
        if (QString::fromStdString(problems[i].getName()) == name &&
            QString::fromStdString(problems[i].getPlatform()) == platform) {
            return i;
        }
    }

    return -1;
}

QString MainWindow::tagsToString(const std::vector<std::string>& tags) {
    QStringList values;
    for (const auto& tag : tags) {
        values << QString::fromStdString(tag);
    }
    return values.join(", ");
}

QString MainWindow::problemToSearchText(const Problem& problem) {
    return QString("%1 %2 %3 %4 %5 %6")
        .arg(QString::fromStdString(problem.getName()))
        .arg(QString::fromStdString(problem.getPlatform()))
        .arg(QString::fromStdString(difficultyToString(problem.getDifficulty())))
        .arg(tagsToString(problem.getTags()))
        .arg(QString::fromStdString(statusToString(problem.getStatus())))
        .arg(QString::fromStdString(problem.getNotes()))
        .toLower();
}

void MainWindow::addProblem() {
    AddProblemDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const AddProblemResult result = manager.addProblem(dialog.problem());
    if (result == AddProblemResult::Duplicate) {
        QMessageBox::warning(this, "Duplicate problem", "This problem already exists for the same platform.");
        return;
    }

    refreshAll();
    autoSave();
}

void MainWindow::deleteSelectedProblem() {
    const int index = selectedSourceIndex();
    if (index < 0) {
        QMessageBox::information(this, "No selection", "Select a problem first.");
        return;
    }

    if (QMessageBox::question(this, "Delete problem", "Delete the selected problem?") != QMessageBox::Yes) {
        return;
    }

    manager.deleteAtIndex(index);
    refreshAll();
    autoSave();
}

void MainWindow::loadFile() {
    const QString path = QFileDialog::getOpenFileName(this, "Open CSV", QString(), "CSV files (*.csv);;All files (*.*)");
    if (!path.isEmpty()) {
        loadFromPath(path);
    }
}

void MainWindow::saveFile() {
    if (currentFile.isEmpty()) {
        saveFileAs();
        return;
    }

    saveToPath(currentFile);
}

void MainWindow::saveFileAs() {
    const QString path = QFileDialog::getSaveFileName(this, "Save CSV", currentFile, "CSV files (*.csv);;All files (*.*)");
    if (!path.isEmpty()) {
        saveToPath(path);
    }
}

void MainWindow::importFile() {
    const QString path = QFileDialog::getOpenFileName(this, "Import CSV", QString(), "CSV files (*.csv);;All files (*.*)");
    if (path.isEmpty()) {
        return;
    }

    std::vector<Problem> problems = manager.getAllProblems();
    ImportResult result = ProblemRepository::importFromFile(path.toStdString(), problems);
    if (!result.fileOpened) {
        QMessageBox::warning(this, "Import failed", "Could not open the selected CSV file.");
        return;
    }

    manager.setProblems(problems);
    refreshAll();
    autoSave();

    QMessageBox::information(this, "CSV imported",
        QString("Imported: %1\nDuplicates skipped: %2\nInvalid rows: %3")
            .arg(result.importedCount)
            .arg(result.duplicateCount)
            .arg(result.invalidCount));
}

void MainWindow::exportFile() {
    const QString path = QFileDialog::getSaveFileName(this, "Export CSV", currentFile, "CSV files (*.csv);;All files (*.*)");
    if (!path.isEmpty() && !ProblemRepository::exportToFile(path.toStdString(), manager.getAllProblems())) {
        QMessageBox::warning(this, "Export failed", "Could not export the CSV file.");
    }
}

void MainWindow::applySearch() {
    const QString query = searchEdit->text().trimmed().toLower();
    if (query.isEmpty()) {
        refreshAll();
        return;
    }

    const std::string queryText = query.toStdString();
    const auto& allProblems = manager.getAllProblems();

    std::vector<Problem> exactMatches;
    for (const Problem& problem : allProblems) {
        if (toLowerCase(problem.getName()) == queryText) {
            exactMatches.push_back(problem);
        }
    }

    if (!exactMatches.empty()) {
        refreshTable(exactMatches);
        statusLabel->setText(QString("%1 exact match%2")
            .arg(exactMatches.size())
            .arg(exactMatches.size() == 1 ? "" : "es"));
        return;
    }

    std::vector<Problem> textMatches;
    for (const Problem& problem : allProblems) {
        if (problemToSearchText(problem).contains(query)) {
            textMatches.push_back(problem);
        }
    }

    if (!textMatches.empty()) {
        refreshTable(textMatches);
        statusLabel->setText(QString("%1 matching problems").arg(textMatches.size()));
        return;
    }

    SearchResult closest = manager.findByName(queryText);
    if (closest.fuzzyFound || closest.exactFound) {
        const Problem* problem = manager.getProblemAt(closest.index);
        if (problem != nullptr) {
            refreshTable({*problem});
            statusLabel->setText(QString("Closest match: %1")
                .arg(QString::fromStdString(closest.closestMatch)));
            return;
        }
    }

    refreshTable({});
    statusLabel->setText("No matching problems");
}

void MainWindow::sortByDifficulty() {
    manager.sortByDifficulty();
    refreshAll();
    autoSave();
}

void MainWindow::sortByTime() {
    manager.sortByTime();
    refreshAll();
    autoSave();
}

void MainWindow::sortByRating() {
    manager.descendingSortByRating();
    refreshAll();
    autoSave();
}










