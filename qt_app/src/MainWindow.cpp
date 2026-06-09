#include "MainWindow.h"
#include "AddProblemDialog.h"
#include "Problem.h"

#include <QApplication>
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
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>

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
    void setDarkTheme(bool dark)
    {
        isDark = dark;
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

        int xOffset = 0;
        int yOffset = 26;
        painter.setPen(isDark ? QColor(205, 214, 244) : QColor(100, 116, 139));
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

                QColor color = isDark ? QColor(49, 50, 68) : QColor(235, 237, 240);
                if (count == 1)
                    color = isDark ? QColor(14, 68, 41) : QColor(155, 233, 168);
                else if (count == 2)
                    color = isDark ? QColor(0, 109, 50) : QColor(64, 196, 99);
                else if (count >= 3)
                    color = isDark ? QColor(38, 166, 65) : QColor(33, 110, 57);

                painter.setBrush(color);
                painter.setPen(Qt::NoPen);
                QRect rect(xOffset + col * (squareSize + spacing), yOffset + row * (squareSize + spacing), squareSize, squareSize);
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

        int xOffset = 0;
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
    bool isDark = false;
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

    QFrame *detailsContainer;
    QLabel *detailsNameLabel;
    QTextEdit *detailsNotesText;

    QPieSeries *pieSeries;
    QChartView *pieChartView;
    HeatmapWidget *heatmap;
    QAction *themeAction;

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
        toolbar->addSeparator();

        themeAction = toolbar->addAction("Dark Mode", mw, &MainWindow::toggleDarkMode);

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

        searchEdit->setPlaceholderText("Search by name, platform, tag... (e.g., diff:hard platform:leetcode)");
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

        table->setColumnCount(9);
        table->setHorizontalHeaderLabels({"Name", "Platform", "Difficulty", "Tags", "Status",
                                          "Time", "Date", "Rating", "Notes"});
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);
        table->verticalHeader()->setVisible(false);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        table->setWordWrap(false);
        table->setShowGrid(false);

        auto header = table->horizontalHeader();
        header->setSectionResizeMode(0, QHeaderView::Interactive);
        header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(3, QHeaderView::Interactive);
        header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(8, QHeaderView::Stretch);

        table->setColumnWidth(0, 200);
        table->setColumnWidth(3, 140);

        detailsContainer = new QFrame(mw);
        detailsContainer->setObjectName("detailsContainer");

        auto *detailsLayout = new QVBoxLayout(detailsContainer);
        detailsLayout->setContentsMargins(16, 12, 16, 12);

        detailsNameLabel = new QLabel(detailsContainer);
        detailsNotesText = new QTextEdit(detailsContainer);
        detailsNotesText->setReadOnly(true);
        detailsNotesText->setMaximumHeight(70);

        detailsLayout->addWidget(detailsNameLabel);
        detailsLayout->addWidget(detailsNotesText);

        auto *tablePanel = new QFrame(mw);
        tablePanel->setObjectName("tablePanel");
        auto *tableLayout = new QVBoxLayout(tablePanel);
        tableLayout->setContentsMargins(1, 1, 1, 1);
        tableLayout->setSpacing(6);
        tableLayout->addWidget(table);
        tableLayout->addWidget(detailsContainer);

        QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, mw, [this, mw]()
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

            QString mainColor = mw->isDarkMode ? "#cdd6f4" : "#0f172a";
            QString subColor = mw->isDarkMode ? "#a6adc8" : "#64748b";
            detailsNameLabel->setText(QString("<span style='color:%1; font-weight:bold; font-size:16px;'>%2</span> <span style='color:%3; font-weight:normal; font-size:13px;'>&nbsp;|&nbsp; %4</span>").arg(mainColor, name, subColor, tags));
            detailsNotesText->setText(notes.isEmpty() ? "No notes added for this problem." : notes);
            detailsContainer->setVisible(true); });

        QWidget *statsContainer = new QWidget(mw);
        statsContainer->setObjectName("statsContainer");

        QVBoxLayout *statsLayout = new QVBoxLayout(statsContainer);
        statsLayout->setContentsMargins(7, 12, 28, 12);
        statsLayout->setSpacing(24);

        statsLabel->setObjectName("statsPanel");
        statsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        statsLabel->setWordWrap(true);

        pieSeries = new QPieSeries();
        pieSeries->setHoleSize(0.35);
        pieSeries->append("Easy", 0);
        pieSeries->append("Medium", 0);
        pieSeries->append("Hard", 0);

        QChart *pieChart = new QChart();
        pieChart->addSeries(pieSeries);
        pieChart->setTitle("Difficulty Split");
        pieChart->setTitleFont(QFont("Arial", 10, QFont::Bold));
        pieChart->legend()->setAlignment(Qt::AlignBottom);
        pieChart->legend()->setFont(QFont("Arial", 8));
        pieChart->setMargins(QMargins(0, 0, 0, 0));
        pieChart->setBackgroundBrush(Qt::transparent);

        pieChartView = new QChartView(pieChart, statsContainer);
        pieChartView->setRenderHint(QPainter::Antialiasing);
        pieChartView->setMinimumHeight(240);
        pieChartView->setVisible(false);
        pieChartView->setFrameShape(QFrame::NoFrame);

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

// ---------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(std::make_unique<MainWindowImpl>()),
      cliProcess(new QProcess(this))
{
    ui->setupUi(this);

    updateThemeStyles();

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

void MainWindow::toggleDarkMode()
{
    isDarkMode = !isDarkMode;
    updateThemeStyles();
    sendCommandToCli("STATS");

    auto rows = ui->table->selectionModel()->selectedRows();
    if (!rows.isEmpty())
    {
        emit ui->table->selectionModel()->selectionChanged(QItemSelection(), QItemSelection());
    }
}

void MainWindow::updateThemeStyles()
{
    if (isDarkMode)
    {
        QString darkCSS = R"(
            QMainWindow, QWidget#statsContainer { background-color: #1e1e2e; }
            QScrollArea#statsScroll { background-color: #1e1e2e; border: none; }
            QWidget { color: #cdd6f4; font-family: "Segoe UI", Arial, sans-serif; }
            
            /* --- STYLE MODERN 2026 PENTRU ADD PROBLEM DIALOG (DARK) --- */
            QDialog { 
                background-color: #11111b; 
                border: 1.5px solid #313244; 
                border-radius: 16px; 
            }
            QDialog QLabel {
                color: #cdd6f4;
                font-weight: 600;
                font-size: 13px;
            }
            QDialog QLabel#dialogTitle {
                font-size: 20px;
                font-weight: 800;
                color: #b4befe;
            }
            QDialog QLabel#dialogSubtitle {
                font-size: 12px;
                color: #a6adc8;
            }
            QDialog QLineEdit, QDialog QComboBox, QDialog QSpinBox, QDialog QDoubleSpinBox, QDialog QDateEdit, QDialog QPlainTextEdit {
                background-color: #1e1e2e;
                color: #cdd6f4;
                border: 1.5px solid #313244;
                border-radius: 10px;
                padding: 8px 12px;
                font-size: 13px;
            }
            QDialog QLineEdit:hover, QDialog QComboBox:hover, QDialog QSpinBox:hover, QDialog QDoubleSpinBox:hover, QDialog QDateEdit:hover, QDialog QPlainTextEdit:hover {
                border-color: #45475a;
                background-color: #252538;
            }
            QDialog QLineEdit:focus, QDialog QComboBox:focus, QDialog QSpinBox:focus, QDialog QDoubleSpinBox:focus, QDialog QDateEdit:focus, QDialog QPlainTextEdit:focus {
                border-color: #b4befe;
                background-color: #1e1e2e;
            }
            QDialog QPushButton {
                background-color: #1e1e2e;
                color: #a6adc8;
                border: 1px solid #313244;
                border-radius: 12px;
                padding: 8px 18px;
                font-weight: 700;
                font-size: 13px;
            }
            QDialog QPushButton:hover {
                background-color: #313244;
                color: #cdd6f4;
                border-color: #45475a;
            }
            QDialog QPushButton:default {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4befe, stop:1 #89b4fa);
                color: #11111b;
                border: none;
            }
            QDialog QPushButton:default:hover {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #cba6f7, stop:1 #b4befe);
            }

            QFrame#metricCard, QFrame#tablePanel, QFrame#detailsContainer {
                background-color: #181825; border: 1px solid #313244; border-radius: 10px;
            }
            QLabel#appTitle, QLabel#dialogTitle { font-size: 22px; font-weight: 800; color: #cdd6f4; }
            QLabel#appSubtitle, QLabel#dialogSubtitle { color: #a6adc8; font-size: 13px; }
            QLabel#metricLabel { color: #bac2de; font-size: 12px; font-weight: bold; }
            QLabel#metricValue { font-size: 24px; font-weight: bold; color: #89b4fa; }
            
            /* Inputs de Bază (Main Window) */
            QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QPlainTextEdit {
                background-color: #181825; color: #cdd6f4; border: 1px solid #313244; 
                border-radius: 8px; padding: 8px 12px; selection-background-color: #89b4fa;
            }
            QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QPlainTextEdit:focus { 
                border: 1px solid #89b4fa; background-color: #1e1e2e;
            }
            
            /* Notes cu lățime maximă redusă */
            QPlainTextEdit#notesEdit {
                max-height: 60px;
                max-width: 350px;
                margin-bottom: 18px;
            }

            QComboBox QAbstractItemView {
                background-color: #11111b; color: #cdd6f4; selection-background-color: #313244; border: 1px solid #313244; border-radius: 8px;
            }
            
            /* --- BUTOANE STYLE 2026 (Pill, Glow, Gradient) --- */
            QPushButton {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #313244, stop:1 #181825);
                color: #cdd6f4; border: 1px solid #45475a; 
                border-radius: 14px; padding: 8px 18px; font-weight: 800; font-size: 13px;
            }
            QPushButton:hover { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #45475a, stop:1 #313244);
                border: 1px solid #89b4fa;
            }
            QPushButton:pressed { background-color: #11111b; }
            
            QPushButton#primaryButton { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #89b4fa, stop:1 #a6e3a1);
                color: #11111b; border: none; min-width: 80px;
            }
            QPushButton#primaryButton:hover { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4befe, stop:1 #94e2d5);
            }
            
            /* Tabel */
            QTableWidget {
                background-color: #181825; alternate-background-color: #212135; color: #cdd6f4; border: none; 
                selection-background-color: #313244; selection-color: #89b4fa; outline: none;
            }
            QTableWidget::viewport { background-color: transparent; border: none; }
            QTableWidget::item { border-bottom: 1px solid #313244; padding: 4px; }
            
            QHeaderView, QHeaderView::section {
                background-color: #11111b; color: #bac2de; border: none; font-weight: bold;
            }
            QHeaderView::section { border-bottom: 2px solid #313244; padding: 6px; }
            QTableCornerButton::section { background-color: #11111b; border: none; }
            
            /* Meniuri */
            QMenuBar { background-color: #1e1e2e; color: #cdd6f4; }
            QMenuBar::item:selected { background-color: #313244; border-radius: 4px;}
            QMenu { background-color: #181825; color: #cdd6f4; border: 1px solid #313244; border-radius: 6px;}
            QMenu::item:selected { background-color: #313244; border-radius: 4px; }
            
            /* Toolbar Pill Buttons */
            QToolBar { background-color: #1e1e2e; border-bottom: 1px solid #313244; spacing: 8px; padding: 4px; }
            QToolBar QToolButton { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2b2d42, stop:1 #181825);
                border: 1px solid #45475a; border-radius: 14px; padding: 4px 14px; color: #cdd6f4; font-weight: bold; 
            }
            QToolBar QToolButton:hover { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #313244, stop:1 #1e1e2e);
                border: 1px solid #89b4fa; 
            }
            QToolBar QToolButton:pressed { background-color: #11111b; }
            
            QStatusBar { background-color: #1e1e2e; color: #bac2de; }
            QTextEdit { background-color: transparent; color: #a6adc8; border: none; }
            
            /* Scrollbars */
            QScrollBar:vertical { background: #1e1e2e; width: 12px; margin: 0px; }
            QScrollBar::handle:vertical { background: #45475a; min-height: 20px; border-radius: 6px; }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
            QScrollBar:horizontal { background: #1e1e2e; height: 12px; margin: 0px; }
            QScrollBar::handle:horizontal { background: #45475a; min-width: 20px; border-radius: 6px; }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }
            QScrollBar::corner { background: transparent; }
            
            QChartView { background: transparent; border: none; }
        )";

        qApp->setStyleSheet(darkCSS);

        if (ui->pieChartView && ui->pieChartView->chart())
        {
            ui->pieChartView->chart()->setTheme(QChart::ChartThemeDark);
            ui->pieChartView->chart()->setBackgroundBrush(Qt::transparent);
            if (ui->pieSeries->slices().size() >= 3)
            {
                ui->pieSeries->slices().at(0)->setBrush(QColor(166, 227, 161));
                ui->pieSeries->slices().at(1)->setBrush(QColor(249, 226, 175));
                ui->pieSeries->slices().at(2)->setBrush(QColor(243, 139, 168));
                for (auto *slice : ui->pieSeries->slices())
                {
                    slice->setBorderColor(QColor(30, 30, 46));
                    slice->setBorderWidth(2);
                }
            }
            ui->pieChartView->chart()->setTitleBrush(QColor(205, 214, 244));
            ui->pieChartView->chart()->legend()->setLabelColor(QColor(186, 194, 222));
        }
        ui->themeAction->setText("Light Mode");
    }
    else
    {
        QString lightCSS = R"(
            QMainWindow, QWidget#statsContainer { background-color: #f8fafc; }
            QScrollArea#statsScroll { background-color: #f8fafc; border: none; }
            QWidget { color: #0f172a; font-family: "Segoe UI", Arial, sans-serif; }
            
            /* --- STYLE MODERN 2026 PENTRU ADD PROBLEM DIALOG (LIGHT) --- */
            QDialog { 
                background-color: #ffffff; 
                border: 1.5px solid #cbd5e1; 
                border-radius: 16px; 
            }
            QDialog QLabel {
                color: #0f172a;
                font-weight: 600;
                font-size: 13px;
            }
            QDialog QLabel#dialogTitle {
                font-size: 20px;
                font-weight: 800;
                color: #2563eb;
            }
            QDialog QLabel#dialogSubtitle {
                font-size: 12px;
                color: #64748b;
            }
            QDialog QLineEdit, QDialog QComboBox, QDialog QSpinBox, QDialog QDoubleSpinBox, QDialog QDateEdit, QDialog QPlainTextEdit {
                background-color: #f8fafc;
                color: #0f172a;
                border: 1.5px solid #e2e8f0;
                border-radius: 10px;
                padding: 8px 12px;
                font-size: 13px;
            }
            QDialog QLineEdit:hover, QDialog QComboBox:hover, QDialog QSpinBox:hover, QDialog QDoubleSpinBox:hover, QDialog QDateEdit:hover, QDialog QPlainTextEdit:hover {
                border-color: #cbd5e1;
                background-color: #f1f5f9;
            }
            QDialog QLineEdit:focus, QDialog QComboBox:focus, QDialog QSpinBox:focus, QDialog QDoubleSpinBox:focus, QDialog QDateEdit:focus, QDialog QPlainTextEdit:focus {
                border-color: #3b82f6;
                background-color: #ffffff;
            }
            QDialog QPushButton {
                background-color: #f8fafc;
                color: #475569;
                border: 1px solid #cbd5e1;
                border-radius: 12px;
                padding: 8px 18px;
                font-weight: 700;
                font-size: 13px;
            }
            QDialog QPushButton:hover {
                background-color: #f1f5f9;
                color: #0f172a;
                border-color: #94a3b8;
            }
            QDialog QPushButton:default {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2563eb, stop:1 #10b981);
                color: #ffffff;
                border: none;
            }
            QDialog QPushButton:default:hover {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3b82f6, stop:1 #34d399);
            }

            QFrame#metricCard, QFrame#tablePanel, QFrame#detailsContainer {
                background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 10px;
            }
            QLabel#appTitle, QLabel#dialogTitle { font-size: 22px; font-weight: 800; color: #0f172a; }
            QLabel#appSubtitle, QLabel#dialogSubtitle { color: #64748b; font-size: 13px; }
            QLabel#metricLabel { color: #64748b; font-size: 12px; font-weight: bold; }
            QLabel#metricValue { font-size: 24px; font-weight: bold; color: #2563eb; }
            
            /* Inputs de Bază (Main Window) */
            QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QPlainTextEdit {
                background-color: #f1f5f9; color: #0f172a; border: 1px solid #cbd5e1; 
                border-radius: 8px; padding: 8px 12px; selection-background-color: #bfdbfe;
            }
            QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QPlainTextEdit:focus { 
                border: 1px solid #3b82f6; background-color: #ffffff;
            }

            /* Notes cu lățime maximă redusă */
            QPlainTextEdit#notesEdit {
                max-height: 60px;
                max-width: 350px;
                margin-bottom: 18px;
            }

            QComboBox QAbstractItemView {
                background-color: #ffffff; color: #0f172a; selection-background-color: #eff6ff; border: 1px solid #cbd5e1; border-radius: 8px;
            }
            
            /* --- BUTOANE STYLE 2026 (Light Mode) --- */
            QPushButton {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffffff, stop:1 #f8fafc);
                color: #0f172a; border: 1px solid #cbd5e1; 
                border-radius: 14px; padding: 8px 18px; font-weight: 800; font-size: 13px;
            }
            QPushButton:hover { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f1f5f9, stop:1 #e2e8f0);
                border: 1px solid #3b82f6; 
            }
            QPushButton:pressed { background-color: #cbd5e1; }
            
            QPushButton#primaryButton { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3b82f6, stop:1 #10b981);
                color: #ffffff; border: none; min-width: 80px;
            }
            QPushButton#primaryButton:hover { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #60a5fa, stop:1 #34d399);
            }
            
            /* Tabel */
            QTableWidget {
                background-color: #ffffff; alternate-background-color: #f1f5f9; color: #1e293b; border: none; 
                selection-background-color: #eff6ff; selection-color: #1e40af; outline: none;
            }
            QTableWidget::viewport { background-color: transparent; border: none; }
            QTableWidget::item { border-bottom: 1px solid #f1f5f9; padding: 4px; }
            
            QHeaderView, QHeaderView::section {
                background-color: #f1f5f9; color: #475569; border: none; font-weight: bold;
            }
            QHeaderView::section { border-bottom: 2px solid #cbd5e1; padding: 6px; }
            QTableCornerButton::section { background-color: #f1f5f9; border: none; }
            
            /* Meniuri */
            QMenuBar { background-color: #f8fafc; color: #0f172a; }
            QMenuBar::item:selected { background-color: #e2e8f0; border-radius: 4px; }
            QMenu { background-color: #ffffff; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 6px; }
            QMenu::item:selected { background-color: #f1f5f9; border-radius: 4px; }
            
            /* Toolbar Pill Buttons */
            QToolBar { background-color: #f8fafc; border-bottom: 1px solid #e2e8f0; spacing: 8px; padding: 4px; }
            QToolBar QToolButton { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ffffff, stop:1 #f1f5f9);
                border: 1px solid #cbd5e1; border-radius: 14px; padding: 4px 14px; color: #0f172a; font-weight: bold; 
            }
            QToolBar QToolButton:hover { 
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #f8fafc, stop:1 #e2e8f0);
                border: 1px solid #3b82f6; 
            }
            QToolBar QToolButton:pressed { background-color: #cbd5e1; }
            
            QStatusBar { background-color: #f8fafc; color: #64748b; }
            QTextEdit { background-color: transparent; color: #475569; border: none; }
            
            /* Scrollbars */
            QScrollBar:vertical { background: #f8fafc; width: 12px; margin: 0px; }
            QScrollBar::handle:vertical { background: #cbd5e1; min-height: 20px; border-radius: 6px; }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
            QScrollBar:horizontal { background: #f8fafc; height: 12px; margin: 0px; }
            QScrollBar::handle:horizontal { background: #cbd5e1; min-width: 20px; border-radius: 6px; }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }
            QScrollBar::corner { background: transparent; }
            
            QChartView { background: transparent; border: none; }
        )";

        qApp->setStyleSheet(lightCSS);

        if (ui->pieChartView && ui->pieChartView->chart())
        {
            ui->pieChartView->chart()->setTheme(QChart::ChartThemeLight);
            ui->pieChartView->chart()->setBackgroundBrush(Qt::transparent);
            if (ui->pieSeries->slices().size() >= 3)
            {
                ui->pieSeries->slices().at(0)->setBrush(QColor(34, 197, 94));
                ui->pieSeries->slices().at(1)->setBrush(QColor(234, 179, 8));
                ui->pieSeries->slices().at(2)->setBrush(QColor(239, 68, 68));
                for (auto *slice : ui->pieSeries->slices())
                {
                    slice->setBorderColor(QColor(255, 255, 255));
                    slice->setBorderWidth(2);
                }
            }
            ui->pieChartView->chart()->setTitleBrush(QColor(15, 23, 42));
            ui->pieChartView->chart()->legend()->setLabelColor(QColor(71, 85, 105));
        }
        ui->themeAction->setText("Dark Mode");
    }

    ui->heatmap->setDarkTheme(isDarkMode);
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

                              QString titleColor = isDarkMode ? "#cdd6f4" : "#0f172a";
                              QString borderCol = isDarkMode ? "#313244" : "#e2e8f0";
                              QString textCol = isDarkMode ? "#bac2de" : "#334155";

                              statsHtmlBuffer = QString("<div style='font-size:18px; font-weight:800; color:%1; margin-bottom:14px; border-bottom:2px solid %2; padding-bottom:4px;'>Quick Details</div><div style='font-size:14px; color:%3; line-height:1.6;'>")
                                                    .arg(titleColor, borderCol, textCol);

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
    {
        QString c = isDarkMode ? "#f38ba8" : "#ef4444";
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>Failed:</b> <span style='color:%1; font-weight:600;'>%2</span></div>").arg(c, val);
    }
    else if (key == "progress")
    {
        QString c = isDarkMode ? "#f9e2af" : "#f59e0b";
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>In progress:</b> <span style='color:%1; font-weight:600;'>%2</span></div>").arg(c, val);
    }
    else if (key == "avg_time")
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>Average time:</b> %1 min</div>").arg(val);
    else if (key == "diff_easy")
        statEasy = val.toInt();
    else if (key == "diff_medium")
        statMedium = val.toInt();
    else if (key == "diff_hard")
        statHard = val.toInt();
    else if (key == "most_used_tag")
    {
        if (isDarkMode)
        {
            statsHtmlBuffer += QString("<div style='margin-top:16px; border-top:1px dashed #313244; padding-top:8px;'><b style='color:#cdd6f4;'>Most used tag:</b></div><div style='display:inline-block; background:#313244; color:#89b4fa; padding:2px 8px; border-radius:4px; font-weight:600; margin-top:4px;'>%1</div>").arg(val);
        }
        else
        {
            statsHtmlBuffer += QString("<div style='margin-top:16px; border-top:1px dashed #e2e8f0; padding-top:8px;'><b style='color:#0f172a;'>Most used tag:</b></div><div style='display:inline-block; background:#e0f2fe; color:#0369a1; padding:2px 8px; border-radius:4px; font-weight:600; margin-top:4px;'>%1</div>").arg(val);
        }
    }
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