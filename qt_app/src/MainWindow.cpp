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
#include <QTextEdit>
#include <QItemSelectionModel>
#include <QDebug>

#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QChart>
#include <QPainter>
#include <QDate>
#include <QToolTip>
#include <QMouseEvent>

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

class HeatmapWidget : public QWidget
{
public:
    HeatmapWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(150);
        setMinimumWidth(240);
        setMouseTracking(true);
    }

    void setData(const QMap<QString, int> &data)
    {
        counts = data;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int squareSize = 12;
        int spacing = 4;
        int columns = 16;
        int rows = 7;

        QDate today = QDate::currentDate();
        QDate start = today.addDays(-(columns * 7 - 1));
        while (start.dayOfWeek() != 1)
            start = start.addDays(-1);

        int xOffset = 6;
        int yOffset = 26;

        painter.setPen(QColor(100, 116, 139));
        QFont font = painter.font();
        font.setPointSize(9);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(xOffset, 14, "Activity (Last 4 months)");

        for (int col = 0; col < columns; ++col)
        {
            for (int row = 0; row < rows; ++row)
            {
                QDate d = start.addDays(col * 7 + row);
                if (d > today)
                    break;

                int count = counts.value(d.toString("dd-MM-yyyy"), 0);
                QColor color = QColor(235, 237, 240);
                if (count == 1)
                    color = QColor(155, 233, 168);
                else if (count == 2)
                    color = QColor(64, 196, 99);
                else if (count >= 3)
                    color = QColor(33, 110, 57);

                painter.setBrush(color);
                painter.setPen(Qt::NoPen);
                QRect rect(xOffset + col * (squareSize + spacing),
                           yOffset + row * (squareSize + spacing),
                           squareSize, squareSize);
                painter.drawRoundedRect(rect, 2, 2);
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        int squareSize = 12;
        int spacing = 4;
        int columns = 16;
        int rows = 7;
        QDate today = QDate::currentDate();
        QDate start = today.addDays(-(columns * 7 - 1));
        while (start.dayOfWeek() != 1)
            start = start.addDays(-1);

        int xOffset = 6;
        int yOffset = 26;
        int col = (event->pos().x() - xOffset) / (squareSize + spacing);
        int row = (event->pos().y() - yOffset) / (squareSize + spacing);

        if (col >= 0 && col < columns && row >= 0 && row < rows)
        {
            QDate d = start.addDays(col * 7 + row);
            if (d <= today)
            {
                int count = counts.value(d.toString("dd-MM-yyyy"), 0);
                QString text = count == 0 ? QString("No problems on %1").arg(d.toString("MMM d, yyyy"))
                                          : QString("%1 problems on %2").arg(count).arg(d.toString("MMM d, yyyy"));
                QToolTip::showText(event->globalPosition().toPoint(), text, this);
                return;
            }
        }
        QToolTip::hideText();
    }

private:
    QMap<QString, int> counts;
};

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

struct MainWindowImpl
{
    QTableWidget *table;
    QLineEdit *searchEdit;
    QLabel *statusLabel;
    QLabel *statsLabel;
    QLabel *totalValue;
    QLabel *solvedValue;
    QLabel *ratingValue;
    QLabel *timeValue;

    // NOU: Pointeri pentru panoul inspector de notițe
    QFrame *detailsContainer;
    QLabel *detailsNameLabel;
    QTextEdit *detailsNotesText;

    QPieSeries *pieSeries;
    QChartView *pieChartView;
    HeatmapWidget *heatmap;

    void setupUi(MainWindow *mw)
    {
        mw->setWindowTitle("AlgoTrack");
        mw->resize(1180, 750);
        mw->setMinimumSize(850, 600);

        table = new QTableWidget(mw);
        searchEdit = new QLineEdit(mw);
        statusLabel = new QLabel(mw);
        statsLabel = new QLabel(mw);
        totalValue = new QLabel("0", mw);
        solvedValue = new QLabel("0", mw);
        ratingValue = new QLabel("0.0", mw);
        timeValue = new QLabel("0m", mw);

        auto *fileMenu = mw->menuBar()->addMenu("&File");
        fileMenu->addAction("Open CSV", mw, &MainWindow::loadFile);
        fileMenu->addAction("Save", mw, &MainWindow::saveFile);
        fileMenu->addAction("Save As", mw, &MainWindow::saveFileAs);
        fileMenu->addSeparator();
        fileMenu->addAction("Import CSV", mw, &MainWindow::importFile);
        fileMenu->addAction("Export CSV", mw, &MainWindow::exportFile);

        auto *sortMenu = mw->menuBar()->addMenu("&Sort");
        sortMenu->addAction("By difficulty", mw, &MainWindow::sortByDifficulty);
        sortMenu->addAction("By time", mw, &MainWindow::sortByTime);
        sortMenu->addAction("By rating", mw, &MainWindow::sortByRating);

        auto *toolbar = mw->addToolBar("Quick actions");
        toolbar->setMovable(false);
        toolbar->addAction("Add", mw, &MainWindow::addProblem);
        toolbar->addAction("Delete", mw, &MainWindow::deleteSelectedProblem);
        toolbar->addSeparator();
        toolbar->addAction("Open", mw, &MainWindow::loadFile);
        toolbar->addAction("Save", mw, &MainWindow::saveFile);

        auto *root = new QWidget(mw);
        auto *rootLayout = new QVBoxLayout(root);
        rootLayout->setContentsMargins(18, 16, 18, 12);
        rootLayout->setSpacing(12);

        auto *topRow = new QHBoxLayout;
        auto *titleBox = new QVBoxLayout;
        auto *title = new QLabel("AlgoTrack", mw);
        title->setObjectName("appTitle");
        auto *subtitle = new QLabel("Problems, progress, and patterns in one calm workspace.", mw);
        subtitle->setObjectName("appSubtitle");
        titleBox->addWidget(title);
        titleBox->addWidget(subtitle);

        auto *addButton = makeCommandButton("Add problem", mw);
        addButton->setObjectName("primaryButton");
        auto *openButton = makeCommandButton("Open CSV", mw);
        auto *saveButton = makeCommandButton("Save", mw);

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
        metrics->addWidget(makeMetricCard("Total", totalValue, mw), 0, 0);
        metrics->addWidget(makeMetricCard("Solved", solvedValue, mw), 0, 1);
        metrics->addWidget(makeMetricCard("Avg rating", ratingValue, mw), 0, 2);
        metrics->addWidget(makeMetricCard("Total time", timeValue, mw), 0, 3);
        rootLayout->addLayout(metrics);

        searchEdit->setPlaceholderText("Search by name, platform, tag, status... (Try diff:hard platform:leetcode)");
        searchEdit->setMinimumHeight(40);

        auto *searchButton = makeCommandButton("Search", mw);
        auto *clearButton = makeCommandButton("Clear", mw);
        auto *sortDifficultyButton = makeCommandButton("Difficulty", mw);
        auto *sortTimeButton = makeCommandButton("Time", mw);
        auto *sortRatingButton = makeCommandButton("Rating", mw);

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

        // ==========================================
        // SMART GRID SYSTEM (Rezolvarea crampelor)
        // ==========================================
        table->setColumnCount(9);
        table->setHorizontalHeaderLabels({"Name", "Platform", "Difficulty", "Tags", "Status",
                                          "Time", "Date", "Rating", "Notes"});
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);
        table->verticalHeader()->setVisible(false);

        // ACTIVAM BARA DE SCROLL ORIZONTALĂ LA NEVOIE
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        table->setWordWrap(false);

        auto header = table->horizontalHeader();
        // Redimensionare inteligentă pe fiecare coloană individual
        header->setSectionResizeMode(0, QHeaderView::Interactive);      // Name: se poate trage manual
        header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Platform: exact cât textul
        header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Difficulty: exact cât textul
        header->setSectionResizeMode(3, QHeaderView::Interactive);      // Tags: manual/proporțional
        header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Status: exact
        header->setSectionResizeMode(5, QHeaderView::ResizeToContents); // Time: exact
        header->setSectionResizeMode(6, QHeaderView::ResizeToContents); // Date: exact
        header->setSectionResizeMode(7, QHeaderView::ResizeToContents); // Rating: exact
        header->setSectionResizeMode(8, QHeaderView::Stretch);          // Notes: absoarbe restul spațiului ecranului

        // Oferim niște lățimi implicite sănătoase coloanelor flexibile
        table->setColumnWidth(0, 200);
        table->setColumnWidth(3, 140);

        // ==========================================
        // PANOU INSPECTOR DETALII (Stil 2026)
        // ==========================================
        detailsContainer = new QFrame(mw);
        detailsContainer->setObjectName("detailsContainer");
        detailsContainer->setStyleSheet(
            "QFrame#detailsContainer { "
            "background-color: #ffffff; border: 1px solid #cbd5e1; "
            "border-radius: 8px; margin-top: 4px; }");
        detailsContainer->setVisible(false); // Ascuns din start

        auto *detailsLayout = new QVBoxLayout(detailsContainer);
        detailsLayout->setContentsMargins(16, 12, 16, 12);

        detailsNameLabel = new QLabel(detailsContainer);
        detailsNameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #0f172a;");

        detailsNotesText = new QTextEdit(detailsContainer);
        detailsNotesText->setReadOnly(true);
        detailsNotesText->setStyleSheet("border: none; background: transparent; color: #475569; font-size: 13px;");
        detailsNotesText->setMaximumHeight(70);

        detailsLayout->addWidget(detailsNameLabel);
        detailsLayout->addWidget(detailsNotesText);

        // Ansamblarea pe partea stângă (Tabel sus + Panou Inspector jos)
        auto *tablePanel = new QFrame(mw);
        tablePanel->setObjectName("tablePanel");
        auto *tableLayout = new QVBoxLayout(tablePanel);
        tableLayout->setContentsMargins(1, 1, 1, 1);
        tableLayout->setSpacing(6);
        tableLayout->addWidget(table);
        tableLayout->addWidget(detailsContainer);

        // Legătura magică: Când dai click pe un rând, actualizăm panoul
        QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, mw, [this]()
                         {
            auto rows = table->selectionModel()->selectedRows();
            if (rows.isEmpty()) {
                detailsContainer->setVisible(false);
                return;
            }
            int r = rows.first().row();
            QString name = table->item(r, 0)->text();
            QString tags = table->item(r, 3)->text();
            QString notes = table->item(r, 8)->text();

            detailsNameLabel->setText(QString("<span style='color:#0f172a;'>%1</span> <span style='color:#64748b; font-weight:normal; font-size:13px;'>&nbsp;|&nbsp; %2</span>").arg(name, tags));
            detailsNotesText->setText(notes.isEmpty() ? "No notes added for this problem." : notes);
            detailsContainer->setVisible(true); });

        // --- Restul UI-ului de grafice rămâne neschimbat ---
        QWidget *statsContainer = new QWidget(mw);
        QVBoxLayout *statsLayout = new QVBoxLayout(statsContainer);
        statsLayout->setContentsMargins(12, 12, 12, 12);
        statsLayout->setSpacing(24);

        statsLabel->setObjectName("statsPanel");
        statsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        statsLabel->setWordWrap(true);

        pieSeries = new QPieSeries();
        pieSeries->setHoleSize(0.35);
        pieSeries->append("Easy", 0)->setBrush(QColor(34, 197, 94));
        pieSeries->append("Medium", 0)->setBrush(QColor(234, 179, 8));
        pieSeries->append("Hard", 0)->setBrush(QColor(239, 68, 68));

        QChart *pieChart = new QChart();
        pieChart->addSeries(pieSeries);
        pieChart->setTitle("Difficulty Split");
        pieChart->setTitleFont(QFont("Arial", 10, QFont::Bold));
        pieChart->legend()->setAlignment(Qt::AlignBottom);
        pieChart->legend()->setFont(QFont("Arial", 8));
        pieChart->setMargins(QMargins(0, 0, 0, 0));
        pieChart->setBackgroundRoundness(0);

        pieChartView = new QChartView(pieChart, statsContainer);
        pieChartView->setRenderHint(QPainter::Antialiasing);
        pieChartView->setMinimumHeight(240);
        pieChartView->setVisible(false);

        heatmap = new HeatmapWidget(statsContainer);

        statsLayout->addWidget(statsLabel);
        statsLayout->addWidget(pieChartView);
        statsLayout->addWidget(heatmap);
        statsLayout->addStretch();

        auto *splitter = new QSplitter(mw);
        auto *statsScroll = new QScrollArea(mw);
        statsScroll->setObjectName("statsScroll");
        statsScroll->setWidget(statsContainer);
        statsScroll->setWidgetResizable(true);
        statsScroll->setFrameShape(QFrame::NoFrame);
        statsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        statsScroll->setMinimumWidth(260);
        statsScroll->setMaximumWidth(310);

        splitter->addWidget(tablePanel);
        splitter->addWidget(statsScroll);
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 0);
        splitter->setSizes({860, 260});
        rootLayout->addWidget(splitter, 1);

        mw->setCentralWidget(root);
        mw->statusBar()->addWidget(statusLabel, 1);

        QObject::connect(addButton, &QPushButton::clicked, mw, &MainWindow::addProblem);
        QObject::connect(openButton, &QPushButton::clicked, mw, &MainWindow::loadFile);
        QObject::connect(saveButton, &QPushButton::clicked, mw, &MainWindow::saveFile);
        QObject::connect(searchButton, &QPushButton::clicked, mw, &MainWindow::applySearch);
        QObject::connect(clearButton, &QPushButton::clicked, mw, [mw]()
                         {
            mw->ui->searchEdit->clear();
            mw->applySearch(); });
        QObject::connect(sortDifficultyButton, &QPushButton::clicked, mw, &MainWindow::sortByDifficulty);
        QObject::connect(sortTimeButton, &QPushButton::clicked, mw, &MainWindow::sortByTime);
        QObject::connect(sortRatingButton, &QPushButton::clicked, mw, &MainWindow::sortByRating);
        QObject::connect(searchEdit, &QLineEdit::returnPressed, mw, &MainWindow::applySearch);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(std::make_unique<MainWindowImpl>()),
      cliProcess(new QProcess(this))
{
    ui->setupUi(this);

    QString appDir = QCoreApplication::applicationDirPath();
    cliProcess->setWorkingDirectory(appDir);

    QString cliPath = "";
    QStringList possiblePaths = {
        appDir + "/AlgoTrackCli", appDir + "/AlgoTrackCli.exe",
        appDir + "/../AlgoTrackCli", appDir + "/../AlgoTrackCli.exe",
        appDir + "/../../AlgoTrackCli", appDir + "/../../AlgoTrackCli.exe"};

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
        ui->statusLabel->setText("Error: Backend not found.");
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

void MainWindow::sendCommandToCli(const QString &command)
{
    if (cliProcess->state() == QProcess::Running)
    {
        cliProcess->write((command + "\n").toUtf8());
    }
}

void MainWindow::handleCliOutput()
{
    while (cliProcess->canReadLine())
    {
        QString line = QString::fromUtf8(cliProcess->readLine()).trimmed();
        if (line.isEmpty())
            continue;
        IPC::Message msg = IPC::parseMessage(line);
        processMessage(msg);
    }
}

void MainWindow::processMessage(const IPC::Message &msg)
{
    std::visit(overloaded{[this](const IPC::Ready &)
                          {
                              sendCommandToCli("LIST");
                              sendCommandToCli("STATS");
                              ui->statusLabel->setText("Connected to backend.");
                          },
                          [this](const IPC::NotFound &)
                          {
                              ui->table->setRowCount(0);
                              ui->statusLabel->setText("Nu s-au găsit probleme pentru căutarea selectată.");
                          },
                          [this](const IPC::FoundExact &m)
                          {
                              populateTableSingleRow(m.csv);
                              ui->statusLabel->setText(QString("Potrivire exactă găsită: %1").arg(m.name.toUpper()));
                          },
                          [this](const IPC::FoundFuzzy &m)
                          {
                              populateTableSingleRow(m.csv);
                              ui->statusLabel->setText(QString("Fuzzy Match găsit (Levenshtein): %1").arg(m.name.toUpper()));
                          },
                          [this](const IPC::StartList &)
                          {
                              readingList = true;
                              ui->table->setRowCount(0);
                          },
                          [this](const IPC::EndList &)
                          {
                              readingList = false;
                              ui->statusLabel->setText(QString("%1 probleme încărcate").arg(ui->table->rowCount()));

                              int calculatedTotalTime = 0;
                              for (int r = 0; r < ui->table->rowCount(); ++r)
                              {
                                  if (ui->table->item(r, 5))
                                      calculatedTotalTime += ui->table->item(r, 5)->text().toInt();
                              }
                              if (calculatedTotalTime > 0)
                                  ui->timeValue->setText(QString("%1m").arg(calculatedTotalTime));
                          },
                          [this](const IPC::StartStats &)
                          {
                              readingStats = true;
                              statsHtmlBuffer = "<div style='font-size:18px; font-weight:800; color:#1e293b; margin-bottom:14px; border-bottom:2px solid #e2e8f0; padding-bottom:4px;'>Quick Details</div><div style='font-size:14px; color:#334155; line-height:1.6;'>";
                              statEasy = statMedium = statHard = 0;
                              statDates.clear();
                          },
                          [this](const IPC::EndStats &)
                          {
                              readingStats = false;
                              statsHtmlBuffer += "</div>";
                              ui->statsLabel->setText(statsHtmlBuffer);

                              ui->pieSeries->slices().at(0)->setValue(statEasy);
                              ui->pieSeries->slices().at(1)->setValue(statMedium);
                              ui->pieSeries->slices().at(2)->setValue(statHard);
                              ui->pieChartView->setVisible((statEasy + statMedium + statHard) > 0);
                              ui->heatmap->setData(statDates);
                          },
                          [this](const IPC::Success &)
                          {
                              sendCommandToCli("LIST");
                              sendCommandToCli("STATS");
                          },
                          [this](const IPC::Error &m)
                          { QMessageBox::critical(this, "Operation Failed", m.message); },
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
    ui->table->clearContents();
    ui->table->setRowCount(0);
    addCsvRowToTable(csv);
    ui->table->viewport()->update();
    ui->table->selectRow(0);
}

void MainWindow::addCsvRowToTable(const QString &csv)
{
    int row = ui->table->rowCount();
    ui->table->insertRow(row);
    QStringList fields = csv.split(',');
    for (int i = 0; i < fields.size() && i < ui->table->columnCount(); ++i)
    {
        QString cleanField = fields[i].trimmed();
        if (cleanField.startsWith('"') && cleanField.endsWith('"'))
        {
            cleanField = cleanField.mid(1, cleanField.size() - 2);
        }
        else
            cleanField.replace("\"", "");
        if (i == 7)
        {
            bool ok;
            double val = cleanField.toDouble(&ok);
            if (ok)
                cleanField = QString::number(val, 'f', 2);
        }
        auto *item = new QTableWidgetItem(cleanField);
        item->setToolTip(cleanField);
        ui->table->setItem(row, i, item);
    }
}

void MainWindow::processStatLine(const QString &line)
{
    QStringList parts = line.split(':');
    if (parts.size() < 2)
        return;
    QString key = parts[0];
    QString val = line.mid(key.length() + 1).trimmed();

    if (key == "total")
        ui->totalValue->setText(val);
    else if (key == "solved")
        ui->solvedValue->setText(val);
    else if (key == "avg_rating")
    {
        bool ok;
        double rVal = val.toDouble(&ok);
        ui->ratingValue->setText(ok ? QString::number(rVal, 'f', 2) : val);
    }
    else if (key == "total_time" && val.toInt() > 0)
        ui->timeValue->setText(val + "m");
    else if (key == "failed")
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>Failed:</b> <span style='color:#ef4444; font-weight:600;'>%1</span></div>").arg(val);
    else if (key == "progress")
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>In progress:</b> <span style='color:#f59e0b; font-weight:600;'>%1</span></div>").arg(val);
    else if (key == "avg_time")
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>Average time:</b> %1 min</div>").arg(val);
    else if (key == "diff_easy")
        statEasy = val.toInt();
    else if (key == "diff_medium")
        statMedium = val.toInt();
    else if (key == "diff_hard")
        statHard = val.toInt();
    else if (key == "date_freq")
    {
        QStringList dparts = val.split('|');
        if (dparts.size() == 2)
            statDates[dparts[0]] = dparts[1].toInt();
    }
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
    int row = ui->table->currentRow();
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
    QString query = ui->searchEdit->text().trimmed();
    if (query.isEmpty())
    {
        sendCommandToCli("LIST");
        sendCommandToCli("STATS");
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