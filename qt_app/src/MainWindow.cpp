#include "MainWindow.h"
#include "AddProblemDialog.h"
#include "Problem.h"

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
#include <QDebug>

// --- FACTORY-ul de mesaje ---
IPC::Message IPC::parseMessage(const QString &line)
{
    if (line == "READY")
        return Ready{};
    if (line == "NOT_FOUND")
        return NotFound{};
    if (line == "START_LIST")
        return StartList{};
    if (line == "END_LIST")
        return EndList{};
    if (line == "START_STATS")
        return StartStats{};
    if (line == "END_STATS")
        return EndStats{};
    if (line.startsWith("SUCCESS"))
        return Success{};
    if (line.startsWith("ERROR"))
        return Error{line.mid(6).trimmed()};

    if (line.startsWith("FOUND_EXACT") || line.startsWith("FOUND_FUZZY"))
    {
        int firstPipe = line.indexOf('|');
        int secondPipe = line.indexOf('|', firstPipe + 1);
        if (firstPipe != -1 && secondPipe != -1)
        {
            QString name = line.mid(firstPipe + 1, secondPipe - firstPipe - 1).trimmed();
            QString csv = line.mid(secondPipe + 1);
            if (line.startsWith("FOUND_EXACT"))
                return FoundExact{name, csv};
            else
                return FoundFuzzy{name, csv};
        }
    }

    return DataLine{line};
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      table(new QTableWidget(this)),
      searchEdit(new QLineEdit(this)),
      statusLabel(new QLabel(this)),
      statsLabel(new QLabel(this)),
      totalValue(new QLabel("0", this)),
      solvedValue(new QLabel("0", this)),
      ratingValue(new QLabel("0.0", this)),
      timeValue(new QLabel("0m", this)),
      cliProcess(new QProcess(this))
{
    buildUi();

    QString appDir = QCoreApplication::applicationDirPath();
    cliProcess->setWorkingDirectory(appDir);

    QString cliPath = "";
    QStringList possiblePaths = {
        appDir + "/AlgoTrackCli",
        appDir + "/AlgoTrackCli.exe",
        appDir + "/../AlgoTrackCli",
        appDir + "/../AlgoTrackCli.exe",
        appDir + "/../../AlgoTrackCli",
        appDir + "/../../AlgoTrackCli.exe"};

    bool found = false;
    for (const QString &path : possiblePaths)
    {
        if (QFile::exists(path))
        {
            cliPath = path;
            found = true;
            break;
        }
    }

    if (!found)
    {
        qWarning() << "Critical: Backend AlgoTrackCli not found near GUI.";
        statusLabel->setText("Error: Backend not found.");
    }
    else
    {
        connect(cliProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::handleCliOutput);
        cliProcess->start(cliPath);
    }
}

MainWindow::~MainWindow()
{
    if (cliProcess->state() == QProcess::Running)
    {
        sendCommandToCli("EXIT");
        cliProcess->waitForFinished(2000);
    }
}

static QPushButton *makeCommandButton(const QString &text, QWidget *parent)
{
    auto *button = new QPushButton(text, parent);
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(38);
    return button;
}

static QFrame *makeMetricCard(const QString &label, QLabel *value, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName("metricCard");

    auto *labelWidget = new QLabel(label, card);
    labelWidget->setObjectName("metricLabel");
    value->setObjectName("metricValue");

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(4);
    layout->addWidget(labelWidget);
    layout->addWidget(value);

    return card;
}

void MainWindow::buildUi()
{
    setWindowTitle("AlgoTrack");
    resize(1180, 700);
    setMinimumSize(780, 520);

    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("Open CSV", this, &MainWindow::loadFile);
    fileMenu->addAction("Save", this, &MainWindow::saveFile);
    fileMenu->addAction("Save As", this, &MainWindow::saveFileAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Import CSV", this, &MainWindow::importFile);
    fileMenu->addAction("Export CSV", this, &MainWindow::exportFile);

    auto *sortMenu = menuBar()->addMenu("&Sort");
    sortMenu->addAction("By difficulty", this, &MainWindow::sortByDifficulty);
    sortMenu->addAction("By time", this, &MainWindow::sortByTime);
    sortMenu->addAction("By rating", this, &MainWindow::sortByRating);

    auto *toolbar = addToolBar("Quick actions");
    toolbar->setMovable(false);
    toolbar->addAction("Add", this, &MainWindow::addProblem);
    toolbar->addAction("Delete", this, &MainWindow::deleteSelectedProblem);
    toolbar->addSeparator();
    toolbar->addAction("Open", this, &MainWindow::loadFile);
    toolbar->addAction("Save", this, &MainWindow::saveFile);

    auto *root = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(18, 16, 18, 12);
    rootLayout->setSpacing(12);

    auto *topRow = new QHBoxLayout;

    auto *titleBox = new QVBoxLayout;
    auto *title = new QLabel("AlgoTrack", this);
    title->setObjectName("appTitle");
    auto *subtitle = new QLabel("Problems, progress, and patterns in one calm workspace.", this);
    subtitle->setObjectName("appSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);

    auto *addButton = makeCommandButton("Add problem", this);
    addButton->setObjectName("primaryButton");
    auto *openButton = makeCommandButton("Open CSV", this);
    auto *saveButton = makeCommandButton("Save", this);

    topRow->addLayout(titleBox, 1);
    topRow->addWidget(openButton);
    topRow->addWidget(saveButton);
    topRow->addWidget(addButton);
    rootLayout->addLayout(topRow);

    auto *metrics = new QGridLayout;
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

    auto *searchButton = makeCommandButton("Search", this);
    auto *clearButton = makeCommandButton("Clear", this);
    auto *sortDifficultyButton = makeCommandButton("Difficulty", this);
    auto *sortTimeButton = makeCommandButton("Time", this);
    auto *sortRatingButton = makeCommandButton("Rating", this);

    auto *searchRow = new QHBoxLayout;
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
    table->setHorizontalHeaderLabels({"Name", "Platform", "Difficulty", "Tags", "Status",
                                      "Time", "Date", "Rating", "Notes"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setWordWrap(false);

    auto *tablePanel = new QFrame(this);
    tablePanel->setObjectName("tablePanel");
    auto *tableLayout = new QVBoxLayout(tablePanel);
    tableLayout->setContentsMargins(1, 1, 1, 1);
    tableLayout->addWidget(table);

    statsLabel->setObjectName("statsPanel");
    statsLabel->setMinimumWidth(220);
    statsLabel->setMaximumWidth(280);
    statsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    statsLabel->setWordWrap(true);

    auto *splitter = new QSplitter(this);
    auto *statsScroll = new QScrollArea(this);
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
    connect(clearButton, &QPushButton::clicked, this, [this]()
            {
        searchEdit->clear();
        sendCommandToCli("LIST");
        sendCommandToCli("STATS"); });
    connect(sortDifficultyButton, &QPushButton::clicked, this, &MainWindow::sortByDifficulty);
    connect(sortTimeButton, &QPushButton::clicked, this, &MainWindow::sortByTime);
    connect(sortRatingButton, &QPushButton::clicked, this, &MainWindow::sortByRating);
    connect(searchEdit, &QLineEdit::returnPressed, this, &MainWindow::applySearch);
}

void MainWindow::sendCommandToCli(const QString &command)
{
    if (cliProcess->state() == QProcess::Running)
    {
        cliProcess->write((command + "\n").toUtf8());
    }
}

// --- WHOLE MESSAGE HANDLING ---
void MainWindow::handleCliOutput()
{
    // Ne asigurăm că procesăm doar mesaje întregi, prevenind erorile de stream
    while (cliProcess->canReadLine())
    {
        QString line = QString::fromUtf8(cliProcess->readLine()).trimmed();
        if (line.isEmpty())
            continue;

        IPC::Message msg = IPC::parseMessage(line);
        processMessage(msg);
    }
}

// --- TYPE-SAFE DISPATCHER ---
void MainWindow::processMessage(const IPC::Message &msg)
{
    std::visit(overloaded{[this](const IPC::Ready &)
                          {
                              sendCommandToCli("LIST");
                              sendCommandToCli("STATS");
                              statusLabel->setText("Connected to backend.");
                          },
                          [this](const IPC::NotFound &)
                          {
                              table->setRowCount(0);
                              statusLabel->setText("Nu s-au găsit probleme pentru căutarea selectată.");
                          },
                          [this](const IPC::FoundExact &m)
                          {
                              populateTableSingleRow(m.csv);
                              statusLabel->setText(QString("Potrivire exactă găsită: %1").arg(m.name.toUpper()));
                          },
                          [this](const IPC::FoundFuzzy &m)
                          {
                              populateTableSingleRow(m.csv);
                              statusLabel->setText(QString("Fuzzy Match găsit (Levenshtein): %1").arg(m.name.toUpper()));
                          },
                          [this](const IPC::StartList &)
                          {
                              readingList = true;
                              table->setRowCount(0);
                          },
                          [this](const IPC::EndList &)
                          {
                              readingList = false;
                              statusLabel->setText(QString("%1 probleme încărcate").arg(table->rowCount()));

                              int calculatedTotalTime = 0;
                              for (int r = 0; r < table->rowCount(); ++r)
                              {
                                  if (table->item(r, 5))
                                      calculatedTotalTime += table->item(r, 5)->text().toInt();
                              }
                              if (calculatedTotalTime > 0)
                                  timeValue->setText(QString("%1m").arg(calculatedTotalTime));
                          },
                          [this](const IPC::StartStats &)
                          {
                              readingStats = true;
                              statsHtmlBuffer = "<div style='font-size:20px; font-weight:800; color:#1e293b; margin-bottom:14px; border-bottom:2px solid #e2e8f0; padding-bottom:4px;'>Details</div><div style='font-size:14px; color:#334155; line-height:1.6;'>";
                          },
                          [this](const IPC::EndStats &)
                          {
                              readingStats = false;
                              statsHtmlBuffer += "</div>";
                              statsLabel->setText(statsHtmlBuffer);
                          },
                          [this](const IPC::Success &)
                          {
                              sendCommandToCli("LIST");
                              sendCommandToCli("STATS");
                          },
                          [this](const IPC::Error &m)
                          {
                              QMessageBox::critical(this, "Operation Failed", m.message);
                          },
                          [this](const IPC::DataLine &m)
                          {
                              if (readingList)
                                  addCsvRowToTable(m.content);
                              else if (readingStats)
                                  processStatLine(m.content);
                          }},
               msg);
}

void MainWindow::populateTableSingleRow(const QString &csv)
{
    table->clearContents();
    table->setRowCount(0);
    addCsvRowToTable(csv);
    table->viewport()->update();
    table->selectRow(0);
}

void MainWindow::addCsvRowToTable(const QString &csv)
{
    int row = table->rowCount();
    table->insertRow(row);
    QStringList fields = csv.split(',');
    for (int i = 0; i < fields.size() && i < table->columnCount(); ++i)
    {
        QString cleanField = fields[i].trimmed();
        if (cleanField.startsWith('"') && cleanField.endsWith('"'))
        {
            cleanField = cleanField.mid(1, cleanField.size() - 2);
        }
        else
        {
            cleanField.replace("\"", "");
        }
        if (i == 7)
        {
            bool ok;
            double val = cleanField.toDouble(&ok);
            if (ok)
                cleanField = QString::number(val, 'f', 2);
        }
        auto *item = new QTableWidgetItem(cleanField);
        item->setToolTip(cleanField);
        table->setItem(row, i, item);
    }
}

void MainWindow::processStatLine(const QString &line)
{
    QStringList parts = line.split(':');
    if (parts.size() != 2)
        return;
    QString key = parts[0];
    QString val = parts[1];

    if (key == "total")
        totalValue->setText(val);
    else if (key == "solved")
        solvedValue->setText(val);
    else if (key == "avg_rating")
    {
        bool ok;
        double rVal = val.toDouble(&ok);
        ratingValue->setText(ok ? QString::number(rVal, 'f', 2) : val);
    }
    else if (key == "total_time" && val.toInt() > 0)
        timeValue->setText(val + "m");
    else if (key == "failed")
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>Failed:</b> <span style='color:#ef4444; font-weight:600;'>%1</span></div>").arg(val);
    else if (key == "progress")
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>In progress:</b> <span style='color:#f59e0b; font-weight:600;'>%1</span></div>").arg(val);
    else if (key == "avg_time")
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>Average time:</b> %1 min</div>").arg(val);
    else if (key == "most_used_tag")
        statsHtmlBuffer += QString("<div style='margin-top:16px; border-top:1px dashed #e2e8f0; padding-top:8px;'><b style='color:#0f172a;'>Most used tag:</b></div><div style='display:inline-block; background:#e0f2fe; color:#0369a1; padding:2px 8px; border-radius:4px; font-weight:600; margin-top:4px;'>%1</div>").arg(val);
    else
        statsHtmlBuffer += QString("<div style='margin-bottom:4px;'><b>%1:</b> %2</div>").arg(key, val);
}

void MainWindow::addProblem()
{
    auto dialog = AddProblemDialog::create(this);
    if (dialog->exec() != QDialog::Accepted)
        return;

    Problem p = dialog->problem();
    sendCommandToCli("ADD " + QString::fromStdString(p.toCSV()));
}

void MainWindow::deleteSelectedProblem()
{
    int row = table->currentRow();
    if (row < 0)
    {
        QMessageBox::information(this, "Nicio selecție", "Selectează o problemă mai întâi.");
        return;
    }
    if (QMessageBox::question(this, "Șterge problemă", "Ești sigur că vrei să ștergi problema?") != QMessageBox::Yes)
        return;

    sendCommandToCli(QString("DELETE_INDEX %1").arg(row));
}

void MainWindow::applySearch()
{
    QString query = searchEdit->text().trimmed();
    if (query.isEmpty())
    {
        sendCommandToCli("LIST");
        return;
    }
    sendCommandToCli("SEARCH " + query);
}

void MainWindow::sortByDifficulty()
{
    sendCommandToCli("SORT_DIFFICULTY");
    sendCommandToCli("LIST");
}
void MainWindow::sortByTime()
{
    sendCommandToCli("SORT_TIME");
    sendCommandToCli("LIST");
}
void MainWindow::sortByRating()
{
    sendCommandToCli("SORT_RATING");
    sendCommandToCli("LIST");
}

void MainWindow::loadFile()
{
    QString path = QFileDialog::getOpenFileName(this, "Open CSV", QString(), "CSV files (*.csv);;All files (*.*)");
    if (!path.isEmpty())
        sendCommandToCli("OPEN " + path);
}

void MainWindow::saveFile() { sendCommandToCli("SAVE"); }
void MainWindow::saveFileAs()
{
    QString path = QFileDialog::getSaveFileName(this, "Save CSV", QString(), "CSV files (*.csv);;All files (*.*)");
    if (!path.isEmpty())
        sendCommandToCli("SAVE_AS " + path);
}

void MainWindow::importFile()
{
    QString path = QFileDialog::getOpenFileName(this, "Import CSV", QString(), "CSV files (*.csv);;All files (*.*)");
    if (!path.isEmpty())
        sendCommandToCli("IMPORT " + path);
}

void MainWindow::exportFile()
{
    QString path = QFileDialog::getSaveFileName(this, "Export CSV", QString(), "CSV files (*.csv);;All files (*.*)");
    if (!path.isEmpty())
        sendCommandToCli("EXPORT " + path);
}