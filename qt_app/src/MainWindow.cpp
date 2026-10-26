#include "MainWindow.h"
#include "AddProblemDialog.h"
#include "Problem.h"
#include "Utils.h"

#include <QApplication>
#include <QAction>
#include <QButtonGroup>
#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDesktopServices>
#include <QEvent>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFontMetrics>
#include <QItemSelection>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QSizePolicy>
#include <QStatusBar>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QVariant>
#include <QUrl>
#include <QVBoxLayout>

#include <cmath>
#include <vector>

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
    explicit HeatmapWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumSize(200, 60);
        setFixedHeight(64);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMouseTracking(true);
        setAutoFillBackground(false);
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

        const Grid g = grid();
        const QDate today = QDate::currentDate();

        painter.setPen(isDark ? QColor(152, 152, 157) : QColor(110, 110, 115));
        QFont font = painter.font();
        font.setPointSize(9);
        font.setWeight(QFont::DemiBold);
        painter.setFont(font);
        painter.drawText(QRect(0, 0, width(), 18), Qt::AlignLeft | Qt::AlignVCenter, "Activity");

        for (int col = 0; col < g.columns; ++col)
        {
            for (int row = 0; row < g.rows; ++row)
            {
                const QDate date = g.start.addDays(col * 7 + row);
                if (date > today)
                    break;

                const int count = counts.value(date.toString("dd-MM-yyyy"), 0);
                QColor color = isDark ? QColor(255, 255, 255, 18) : QColor("#E5E5EA");
                if (count == 1)
                    color = isDark ? QColor("#67E8F9") : QColor("#7DD3FC");
                else if (count == 2)
                    color = isDark ? QColor("#06B6D4") : QColor("#0284C7");
                else if (count > 2)
                    color = isDark ? QColor("#818CF8") : QColor("#4F46E5");

                painter.setBrush(color);
                painter.setPen(Qt::NoPen);
                const QRect rect(g.x + col * (g.square + g.gap), g.y + row * (g.square + g.gap), g.square, g.square);
                painter.drawRoundedRect(rect, 3, 3);
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        const Grid g = grid();
        const QDate today = QDate::currentDate();
        const int col = (event->pos().x() - g.x) / (g.square + g.gap);
        const int row = (event->pos().y() - g.y) / (g.square + g.gap);

        if (col >= 0 && col < g.columns && row >= 0 && row < g.rows)
        {
            const QDate date = g.start.addDays(col * 7 + row);
            if (date <= today)
            {
                const int count = counts.value(date.toString("dd-MM-yyyy"), 0);
                const QString text = count == 0 ? QString("No problems on %1").arg(date.toString("MMM d, yyyy"))
                                                : QString("%1 problems on %2").arg(count).arg(date.toString("MMM d, yyyy"));
                QToolTip::showText(event->globalPosition().toPoint(), text, this);
                return;
            }
        }
        QToolTip::hideText();
    }

    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange)
            update();
        QWidget::changeEvent(event);
    }

private:
    struct Grid
    {
        int square = 6;
        int gap = 2;
        int columns = 4;
        int rows = 7;
        int x = 0;
        int y = 16;
        QDate start;
    };

    Grid grid() const
    {
        Grid g;
        const int labelHeight = 14;
        const int availH = qMax(1, height() - labelHeight - 2);
        g.square = (availH - (g.rows - 1) * g.gap) / g.rows;
        g.square = qBound(4, g.square, 8);
        g.y = labelHeight;
        const QDate today = QDate::currentDate();
        QDate weekStart = today;
        while (weekStart.dayOfWeek() != Qt::Monday)
            weekStart = weekStart.addDays(-1);
        g.start = weekStart.addDays(-7 * (g.columns - 1));
        return g;
    }

    QMap<QString, int> counts;
    bool isDark = true;
};

class DifficultyDonut : public QWidget
{
public:
    explicit DifficultyDonut(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(188);
        setMaximumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setCounts(int easyCount, int mediumCount, int hardCount, int solvedCount, int totalCount)
    {
        easy = qMax(0, easyCount);
        medium = qMax(0, mediumCount);
        hard = qMax(0, hardCount);
        solved = qMax(0, solvedCount);
        total = qMax(0, totalCount);
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

        const QColor easyColor = isDark ? QColor("#06B6D4") : QColor("#0284C7");
        const QColor mediumColor = isDark ? QColor("#8B5CF6") : QColor("#7C3AED");
        const QColor hardColor = isDark ? QColor("#EF4444") : QColor("#E11D48");
        const QColor track = isDark ? QColor(255, 255, 255, 28) : QColor("#E5E5EA");
        const QColor primary = isDark ? QColor("#F5F5F7") : QColor("#1C1C1E");
        const QColor secondary = isDark ? QColor("#98989D") : QColor("#6E6E73");

        constexpr int legendHeight = 22;
        const int side = qMax(64, qMin(width() - 8, height() - legendHeight - 6));
        QRectF ring((width() - side) / 2.0, 2, side, side);
        ring.adjust(10, 10, -10, -10);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(track, 12, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(ring, 0, 360 * 16);

        struct Slice
        {
            int value;
            QColor color;
        };
        std::vector<Slice> slices;
        if (easy > 0)
            slices.push_back({easy, easyColor});
        if (medium > 0)
            slices.push_back({medium, mediumColor});
        if (hard > 0)
            slices.push_back({hard, hardColor});

        const int sum = easy + medium + hard;
        if (sum > 0 && !slices.empty())
        {
            constexpr int full = 360 * 16;
            const int gap = slices.size() == 1 ? 0 : 16 * 16;
            const int available = full - gap * static_cast<int>(slices.size());
            std::vector<int> spans(slices.size(), 0);
            int consumed = 0;
            int largest = 0;
            for (int i = 0; i < static_cast<int>(slices.size()); ++i)
            {
                spans[static_cast<size_t>(i)] = available * slices[static_cast<size_t>(i)].value / sum;
                consumed += spans[static_cast<size_t>(i)];
                if (slices[static_cast<size_t>(i)].value > slices[static_cast<size_t>(largest)].value)
                    largest = i;
            }
            spans[static_cast<size_t>(largest)] += available - consumed;

            int start = 90 * 16;
            for (int i = 0; i < static_cast<int>(slices.size()); ++i)
            {
                const int span = spans[static_cast<size_t>(i)];
                if (span <= 0)
                    continue;
                painter.setPen(QPen(slices[static_cast<size_t>(i)].color, 12, Qt::SolidLine, Qt::RoundCap));
                painter.drawArc(ring, start, -span);
                start -= span + gap;
            }
        }

        const int library = total > 0 ? total : sum;
        const int percent = library == 0 ? 0 : static_cast<int>(std::lround(100.0 * solved / static_cast<double>(library)));
        const QRectF hole = ring.adjusted(18, 14, -18, -14);

        QFont numberFont = painter.font();
        numberFont.setPixelSize(22);
        numberFont.setWeight(QFont::Bold);
        painter.setFont(numberFont);
        painter.setPen(primary);
        painter.drawText(hole.adjusted(0, 0, 0, -hole.height() / 2.0), Qt::AlignHCenter | Qt::AlignBottom, QString::number(solved));

        QFont subFont = painter.font();
        subFont.setPixelSize(11);
        subFont.setWeight(QFont::DemiBold);
        painter.setFont(subFont);
        painter.setPen(secondary);
        painter.drawText(hole.adjusted(0, hole.height() / 2.0 - 2, 0, 0), Qt::AlignHCenter | Qt::AlignTop,
                         QString("%1% solved").arg(percent));

        QFont legendFont = painter.font();
        legendFont.setPixelSize(11);
        legendFont.setWeight(QFont::Bold);
        painter.setFont(legendFont);
        const QFontMetrics metrics(legendFont);
        struct Legend
        {
            QString text;
            QColor color;
        };
        const Legend legends[] = {
            {QString("Easy %1").arg(easy), easyColor},
            {QString("Med %1").arg(medium), mediumColor},
            {QString("Hard %1").arg(hard), hardColor},
        };
        int legendWidth = 0;
        for (const Legend &item : legends)
            legendWidth += 14 + metrics.horizontalAdvance(item.text) + 14;
        int x = qMax(0, (width() - legendWidth) / 2);
        const int y = height() - legendHeight;
        for (const Legend &item : legends)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(item.color);
            painter.drawEllipse(QRectF(x, y + 4, 8, 8));
            painter.setPen(secondary);
            const QRect textRect(x + 12, y, metrics.horizontalAdvance(item.text) + 4, 16);
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, item.text);
            x = textRect.right() + 14;
        }
    }

    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange)
            update();
        QWidget::changeEvent(event);
    }

private:
    int easy = 0;
    int medium = 0;
    int hard = 0;
    int solved = 0;
    int total = 0;
    bool isDark = true;
};

class LibraryHeader : public QHeaderView
{
public:
    explicit LibraryHeader(QWidget *parent = nullptr) : QHeaderView(Qt::Horizontal, parent)
    {
        setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        setHighlightSections(false);
        setSectionsClickable(false);
        setStretchLastSection(false);
        setFixedHeight(36);
        setMinimumSectionSize(64);
    }

    void setDarkTheme(bool dark)
    {
        isDark = dark;
        viewport()->update();
    }

protected:
    void changeEvent(QEvent *event) override
    {
        if (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange)
            viewport()->update();
        QHeaderView::changeEvent(event);
    }

    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override
    {
        if (!rect.isValid())
            return;
        painter->save();
        const QColor line = isDark ? QColor(255, 255, 255, 28) : QColor(0, 0, 0, 22);
        painter->setPen(QPen(line, 1));
        painter->drawLine(rect.bottomLeft(), rect.bottomRight());

        QString text;
        if (model())
            text = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString().toUpper();

        QFont font = painter->font();
        font.setPixelSize(11);
        font.setWeight(QFont::Bold);
        font.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
        painter->setFont(font);
        painter->setPen(isDark ? QColor(235, 235, 245, 160) : QColor("#6E6E73"));
        const QFontMetrics metrics(font);
        painter->drawText(rect.adjusted(8, 0, -6, -1), Qt::AlignLeft | Qt::AlignVCenter,
                          metrics.elidedText(text, Qt::ElideRight, qMax(0, rect.width() - 14)));
        painter->restore();
    }

private:
    bool isDark = true;
};

class RowHoverWatcher : public QObject
{
public:
    explicit RowHoverWatcher(QTableWidget *table) : QObject(table), table(table)
    {
        table->setProperty("hoverRow", -1);
        table->viewport()->setProperty("hoverRow", -1);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == table->viewport() && event->type() == QEvent::MouseMove)
        {
            const auto *mouse = static_cast<QMouseEvent *>(event);
            setHover(table->rowAt(static_cast<int>(mouse->position().y())));
        }
        else if (watched == table->viewport() && event->type() == QEvent::Leave)
            setHover(-1);
        return QObject::eventFilter(watched, event);
    }

private:
    void setHover(int row)
    {
        if (row == hoverRow)
            return;
        hoverRow = row;
        table->setProperty("hoverRow", hoverRow);
        table->viewport()->setProperty("hoverRow", hoverRow);
        table->viewport()->update();
    }

    QTableWidget *table;
    int hoverRow = -1;
};

static QIcon makeGlyphIcon(const QString &kind, const QColor &color)
{
    const qreal dpr = qMax(1.0, qApp->devicePixelRatio());
    constexpr int logical = 16;
    QPixmap pix(qRound(logical * dpr), qRound(logical * dpr));
    pix.setDevicePixelRatio(dpr);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    if (kind == "search")
    {
        painter.drawEllipse(QRectF(1.2, 1.2, 8.8, 8.8));
        painter.drawLine(QPointF(9.0, 9.0), QPointF(14.2, 14.2));
    }
    else
    {
        painter.drawLine(QPointF(4.2, 4.2), QPointF(11.8, 11.8));
        painter.drawLine(QPointF(11.8, 4.2), QPointF(4.2, 11.8));
    }
    return QIcon(pix);
}

static QPalette glassPalette(bool dark)
{
    QPalette palette;
    if (dark)
    {
        const QColor text(245, 245, 247);
        const QColor disabled(152, 152, 157);
        palette.setColor(QPalette::Window, QColor(16, 16, 18));
        palette.setColor(QPalette::WindowText, text);
        palette.setColor(QPalette::Base, QColor(28, 28, 30));
        palette.setColor(QPalette::AlternateBase, QColor(44, 44, 46));
        palette.setColor(QPalette::ToolTipBase, QColor(44, 44, 46));
        palette.setColor(QPalette::ToolTipText, text);
        palette.setColor(QPalette::Text, text);
        palette.setColor(QPalette::Button, QColor(44, 44, 46));
        palette.setColor(QPalette::ButtonText, text);
        palette.setColor(QPalette::BrightText, QColor("#FF453A"));
        palette.setColor(QPalette::Link, QColor("#0A84FF"));
        palette.setColor(QPalette::Highlight, QColor(10, 132, 255, 80));
        palette.setColor(QPalette::HighlightedText, text);
        palette.setColor(QPalette::PlaceholderText, QColor(235, 235, 245, 120));
        palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
        palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    }
    else
    {
        const QColor text(28, 28, 30);
        const QColor disabled(142, 142, 147);
        palette.setColor(QPalette::Window, QColor(244, 244, 246));
        palette.setColor(QPalette::WindowText, text);
        palette.setColor(QPalette::Base, Qt::white);
        palette.setColor(QPalette::AlternateBase, QColor(242, 242, 247));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, text);
        palette.setColor(QPalette::Text, text);
        palette.setColor(QPalette::Button, Qt::white);
        palette.setColor(QPalette::ButtonText, text);
        palette.setColor(QPalette::BrightText, QColor("#D70015"));
        palette.setColor(QPalette::Link, QColor("#007AFF"));
        palette.setColor(QPalette::Highlight, QColor(0, 122, 255, 40));
        palette.setColor(QPalette::HighlightedText, text);
        palette.setColor(QPalette::PlaceholderText, QColor(60, 60, 67, 120));
        palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
        palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    }
    return palette;
}

static void fitBadgeColumns(QTableWidget *table)
{
    for (int column : {1, 2, 4, 5, 6, 7, 12})
        table->resizeColumnToContents(column);
}

static QFrame *makePanel(const QString &name, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setObjectName(name);
    frame->setFrameShape(QFrame::NoFrame);
    frame->setAttribute(Qt::WA_StyledBackground, true);
    return frame;
}

static QPushButton *makeButton(const QString &text, const QString &name, QWidget *parent)
{
    auto *button = new QPushButton(text, parent);
    button->setObjectName(name);
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(36);
    return button;
}

static QFrame *makeMetricCard(const QString &label, QLabel *value, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName("metricCard");
    card->setFrameShape(QFrame::NoFrame);
    card->setAttribute(Qt::WA_StyledBackground, true);
    auto *labelWidget = new QLabel(label, card);
    labelWidget->setObjectName("metricLabel");
    value->setObjectName("metricValue");
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(2);
    layout->addWidget(labelWidget);
    layout->addWidget(value);
    return card;
}

class ProblemBadgeDelegate : public QStyledItemDelegate
{
public:
    explicit ProblemBadgeDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void setDarkTheme(bool dark) { isDark = dark; }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        const QString raw = index.data(Qt::DisplayRole).toString();
        QFont font = option.font;
        const bool badgeColumn = index.column() == 1 || index.column() == 2 || index.column() == 3 || index.column() == 4;
        font.setPointSize(index.column() == 0 ? 10 : badgeColumn ? 9 : 8);
        font.setWeight((index.column() == 0 || index.column() == 1 || index.column() == 2 || index.column() == 3 || index.column() == 4)
                           ? QFont::Bold
                           : QFont::Normal);
        const QFontMetrics metrics(font);
        int width = 72;
        switch (index.column())
        {
        case 0: {
            const QString notes = index.siblingAtColumn(8).data(Qt::DisplayRole).toString().simplified();
            width = qMin(240, qMax(metrics.horizontalAdvance(raw), metrics.horizontalAdvance(notes)) + 24);
            break;
        }
        case 1:
            width = metrics.horizontalAdvance(raw.trimmed()) + 40;
            break;
        case 2:
            width = metrics.horizontalAdvance(prettyEnum(raw)) + 32;
            break;
        case 4:
            width = metrics.horizontalAdvance(QStringLiteral("● ") + prettyEnum(raw)) + 32;
            break;
        case 3: {
            int tagsWidth = 16;
            const auto tags = raw.contains('|') ? raw.split('|', Qt::SkipEmptyParts) : raw.split(',', Qt::SkipEmptyParts);
            for (const QString &tag : tags)
                tagsWidth += metrics.horizontalAdvance(tag.trimmed()) + 22;
            width = qMin(280, tagsWidth);
            break;
        }
        case 5:
            width = metrics.horizontalAdvance(raw.trimmed() + QStringLiteral(" min")) + 28;
            break;
        case 6:
            width = metrics.horizontalAdvance(raw.trimmed()) + 28;
            break;
        case 7:
            width = metrics.horizontalAdvance(prettyRating(raw)) + 28;
            break;
        case 12:
            width = metrics.horizontalAdvance(raw.trimmed()) + 28;
            break;
        default:
            break;
        }
        return QSize(qMax(64, width), 52);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const int hoverRow = hoveredRowOf(option);
        const bool rowHovered = hoverRow == index.row();
        QColor background(0, 0, 0, 0);
        if (option.state & QStyle::State_Selected)
            background = isDark ? QColor(10, 132, 255, rowHovered ? 64 : 46)
                                : QColor(0, 122, 255, rowHovered ? 36 : 26);
        else if (rowHovered)
            background = isDark ? QColor(255, 255, 255, 18) : QColor(0, 0, 0, 12);
        if (background.alpha() > 0)
            painter->fillRect(option.rect, background);

        const QString raw = index.data(Qt::DisplayRole).toString();
        const QRect cell = option.rect.adjusted(8, 6, -8, -6);
        const QColor primary = isDark ? QColor("#F5F5F7") : QColor("#1C1C1E");
        const QColor secondary = isDark ? QColor(235, 235, 245, 150) : QColor("#6E6E73");

        QFont base = option.font;
        base.setPointSize(10);
        base.setWeight(QFont::Normal);

        switch (index.column())
        {
        case 0:
            paintName(painter, cell, raw, index.siblingAtColumn(8).data(Qt::DisplayRole).toString().simplified(), base, primary, secondary);
            break;
        case 1:
            paintToken(painter, cell, raw.trimmed(), toneFor(QString()), base, false, 1.0);
            break;
        case 2:
            paintToken(painter, cell, prettyEnum(raw), toneFor(raw), base, false, 1.5);
            break;
        case 3:
            paintTags(painter, cell, raw, base);
            break;
        case 4:
            paintToken(painter, cell, prettyEnum(raw), toneFor(raw), base, true, 1.5);
            break;
        case 5:
            paintPlain(painter, cell, raw.trimmed().isEmpty() ? raw.trimmed() : raw.trimmed() + QStringLiteral(" min"), base, secondary);
            break;
        case 6:
            paintPlain(painter, cell, raw.trimmed(), base, secondary);
            break;
        case 7:
            paintPlain(painter, cell, prettyRating(raw), base, primary);
            break;
        case 12:
            paintPlain(painter, cell, raw.trimmed(), base, secondary);
            break;
        default:
            paintPlain(painter, cell, raw.simplified(), base, secondary);
            break;
        }

        QPen rowLine(isDark ? QColor(255, 255, 255, 22) : QColor(0, 0, 0, 18), 1);
        rowLine.setCosmetic(true);
        painter->setPen(rowLine);
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
        painter->restore();
    }

private:
    struct Tone
    {
        QColor fg;
        QColor bg;
        QColor border;
    };

    static int hoveredRowOf(const QStyleOptionViewItem &option)
    {
        const QWidget *widget = option.widget;
        while (widget)
        {
            const QVariant value = widget->property("hoverRow");
            if (value.isValid())
                return value.toInt();
            widget = widget->parentWidget();
        }
        return -1;
    }

    static QString prettyEnum(const QString &raw)
    {
        const QString key = raw.trimmed().toLower();
        if (key == "easy")
            return "Easy";
        if (key == "medium")
            return "Medium";
        if (key == "hard")
            return "Hard";
        if (key == "solved")
            return "Solved";
        if (key == "failed")
            return "Failed";
        if (key == "in_progress" || key == "in progress" || key == "inprogress")
            return "In progress";
        return raw.trimmed();
    }

    static QString prettyRating(const QString &raw)
    {
        bool ok = false;
        const double value = raw.toDouble(&ok);
        return ok ? QString::number(value, 'f', 1) : raw.trimmed();
    }

    Tone toneFor(const QString &raw) const
    {
        const QString key = raw.trimmed().toLower();
        if (key == "easy")
            return isDark ? Tone{QColor("#67E8F9"), QColor("#083344"), QColor("#06B6D4")}
                          : Tone{QColor("#0369A1"), QColor("#E0F2FE"), QColor("#0284C7")};
        if (key == "medium")
            return isDark ? Tone{QColor("#DDD6FE"), QColor("#2E1065"), QColor("#8B5CF6")}
                          : Tone{QColor("#5B21B6"), QColor("#F3E8FF"), QColor("#7C3AED")};
        if (key == "hard" || key == "failed")
            return isDark ? Tone{QColor("#FCA5A5"), QColor("#450A0A"), QColor("#EF4444")}
                          : Tone{QColor("#9F1239"), QColor("#FFE4E6"), QColor("#E11D48")};
        if (key == "solved")
            return isDark ? Tone{QColor("#DBEAFE"), QColor("#1E3A8A"), QColor("#3B82F6")}
                          : Tone{QColor("#1E40AF"), QColor("#DBEAFE"), QColor("#2563EB")};
        if (key == "in_progress" || key == "in progress" || key == "inprogress")
            return isDark ? Tone{QColor("#FFEDD5"), QColor("#431407"), QColor("#F97316")}
                          : Tone{QColor("#9A3412"), QColor("#FFEDD5"), QColor("#EA580C")};
        return isDark ? Tone{QColor(235, 235, 245, 220), QColor(255, 255, 255, 16), QColor(255, 255, 255, 46)}
                      : Tone{QColor("#3A3A3C"), QColor(0, 0, 0, 10), QColor(0, 0, 0, 36)};
    }

    void paintPlain(QPainter *painter, const QRect &rect, const QString &text, const QFont &font, const QColor &color) const
    {
        painter->setFont(font);
        painter->setPen(color);
        const QFontMetrics metrics(font);
        painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, metrics.elidedText(text, Qt::ElideRight, rect.width()));
    }

    void paintName(QPainter *painter, const QRect &rect, const QString &name, const QString &notes, const QFont &base, const QColor &primary, const QColor &secondary) const
    {
        QFont nameFont = base;
        nameFont.setWeight(QFont::DemiBold);
        if (notes.isEmpty())
        {
            paintPlain(painter, rect, name, nameFont, primary);
            return;
        }

        QFont noteFont = base;
        noteFont.setPointSize(8);
        const QRect titleRect(rect.left(), rect.top(), rect.width(), rect.height() / 2 + 2);
        const QRect noteRect(rect.left(), titleRect.bottom() - 1, rect.width(), rect.bottom() - titleRect.bottom() + 1);
        paintPlain(painter, titleRect, name, nameFont, primary);
        paintPlain(painter, noteRect, notes, noteFont, secondary);
    }

    void paintToken(QPainter *painter, const QRect &rect, const QString &text, const Tone &tone, const QFont &base, bool bullet, qreal borderWidth) const
    {
        if (text.isEmpty())
            return;
        QFont font = base;
        font.setPointSize(9);
        font.setWeight(QFont::Bold);
        const QString label = bullet ? QStringLiteral("● ") + text : text;
        drawPill(painter, rect, label, tone, font, 22, borderWidth);
    }

    void paintTags(QPainter *painter, const QRect &rect, const QString &raw, const QFont &base) const
    {
        QStringList tags = raw.split('|', Qt::SkipEmptyParts);
        if (tags.size() <= 1 && raw.contains(','))
            tags = raw.split(',', Qt::SkipEmptyParts);

        QFont font = base;
        font.setPointSize(9);
        font.setWeight(QFont::Bold);
        const Tone tone = toneFor(QString());
        const QFontMetrics metrics(font);
        int x = rect.left();
        const int height = 20;
        const int y = rect.center().y() - height / 2;

        for (const QString &tag : tags)
        {
            const QString label = tag.trimmed();
            if (label.isEmpty())
                continue;
            const int width = metrics.horizontalAdvance(label) + 16;
            if (x > rect.left() && x + width > rect.right())
                break;
            const QRect pill(x, y, qMin(width, rect.right() - x), height);
            drawPill(painter, pill, label, tone, font, height, 1.0);
            x += width + 6;
        }
    }

    static void drawPill(QPainter *painter, const QRect &rect, const QString &text, const Tone &tone, const QFont &font, int height, qreal borderWidth)
    {
        if (text.isEmpty() || rect.width() < 8)
            return;
        painter->setFont(font);
        const QFontMetrics metrics(font);
        constexpr int padX = 8;
        const int available = qMax(8, rect.width());
        const int width = qMin(metrics.horizontalAdvance(text) + padX * 2, available);
        const int pillHeight = qMin(height, qMax(1, rect.height()));
        const QRect pill(rect.left(), rect.center().y() - pillHeight / 2, width, pillHeight);
        painter->setPen(QPen(tone.border, borderWidth));
        painter->setBrush(tone.bg);
        painter->drawRoundedRect(QRectF(pill).adjusted(0.5, 0.5, -0.5, -0.5), 10, 10);
        painter->setPen(tone.fg);
        const QRect textRect = pill.adjusted(padX, 0, -padX, 0);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, metrics.elidedText(text, Qt::ElideRight, qMax(0, textRect.width())));
    }

    bool isDark = true;
};

struct MainWindowImpl
{
    QTableWidget *table = nullptr;
    QLineEdit *searchEdit = nullptr;
    QLabel *statusLabel = nullptr;
    QLabel *statsLabel = nullptr;
    QLabel *totalValue = nullptr;
    QLabel *solvedValue = nullptr;
    QLabel *ratingValue = nullptr;
    QLabel *timeValue = nullptr;

    QFrame *detailsContainer = nullptr;
    QLabel *detailsNameLabel = nullptr;
    QTextEdit *detailsNotesText = nullptr;
    QLabel *detailsMeta = nullptr;
    QLabel *detailsPath = nullptr;
    QWidget *detailsBody = nullptr;
    QToolButton *detailsToggle = nullptr;

    DifficultyDonut *difficultyChart = nullptr;
    LibraryHeader *libraryHeader = nullptr;
    HeatmapWidget *heatmap = nullptr;
    QLabel *timerReadout = nullptr;
    QPushButton *timerToggle = nullptr;
    QPushButton *timerReset = nullptr;
    QAction *themeAction = nullptr;
    QPushButton *themeButton = nullptr;
    QAction *searchGlyph = nullptr;
    QAction *clearGlyph = nullptr;
    ProblemBadgeDelegate *badges = nullptr;
    QPushButton *dueButton = nullptr;
    QButtonGroup *filterGroup = nullptr;
    QLabel *dueLabel = nullptr;
    QPushButton *openUrlButton = nullptr;
    QPushButton *openSolutionButton = nullptr;
    QPushButton *markReviewedButton = nullptr;

    void setupUi(MainWindow *mw)
    {
        mw->setWindowTitle("AlgoTrack");
        mw->resize(1320, 840);
        mw->setMinimumSize(1080, 700);

        table = new QTableWidget(mw);
        searchEdit = new QLineEdit(mw);
        statusLabel = new QLabel(mw);
        statsLabel = new QLabel(mw);
        totalValue = new QLabel("0", mw);
        solvedValue = new QLabel("0", mw);
        ratingValue = new QLabel("0.0", mw);
        timeValue = new QLabel("0m", mw);
        statusLabel->setObjectName("statusLabel");

        auto *fileMenu = mw->menuBar()->addMenu("&File");
        auto *openAction = fileMenu->addAction("Open CSV", mw, &MainWindow::loadFile);
        openAction->setShortcut(QKeySequence::Open);
        auto *saveAction = fileMenu->addAction("Save", mw, &MainWindow::saveFile);
        saveAction->setShortcut(QKeySequence::Save);
        fileMenu->addAction("Save As", mw, &MainWindow::saveFileAs);
        fileMenu->addSeparator();
        fileMenu->addAction("Import CSV", mw, &MainWindow::importFile);
        fileMenu->addAction("Export CSV", mw, &MainWindow::exportFile);
        fileMenu->addAction("Export Markdown", mw, &MainWindow::exportMarkdown);

        auto *sortMenu = mw->menuBar()->addMenu("&Sort");
        sortMenu->addAction("By difficulty", mw, &MainWindow::sortByDifficulty);
        sortMenu->addAction("By time", mw, &MainWindow::sortByTime);
        sortMenu->addAction("By rating", mw, &MainWindow::sortByRating);

        themeAction = new QAction(mw->isDarkMode ? "Light Mode" : "Dark Mode", mw);
        QObject::connect(themeAction, &QAction::triggered, mw, &MainWindow::toggleDarkMode);

        searchEdit->setObjectName("searchField");
        searchEdit->setFrame(false);
        searchEdit->setPlaceholderText("Search problems");
        searchEdit->setClearButtonEnabled(false);
        searchEdit->setMinimumHeight(36);
        const QColor glyph = mw->isDarkMode ? QColor(235, 235, 245, 170) : QColor(60, 60, 67, 150);
        searchGlyph = searchEdit->addAction(makeGlyphIcon("search", glyph), QLineEdit::LeadingPosition);
        searchGlyph->setToolTip("Search");
        clearGlyph = searchEdit->addAction(makeGlyphIcon("clear", glyph), QLineEdit::TrailingPosition);
        clearGlyph->setToolTip("Clear");
        clearGlyph->setVisible(false);

        auto *searchShell = makePanel("searchShell", mw);
        auto *searchLayout = new QHBoxLayout(searchShell);
        searchLayout->setContentsMargins(8, 0, 6, 0);
        searchLayout->addWidget(searchEdit);
        QObject::connect(qApp, &QApplication::focusChanged, searchShell, [searchShell, this](QWidget *, QWidget *now) {
            const bool focused = now == searchEdit;
            if (searchShell->property("focused").toBool() == focused)
                return;
            searchShell->setProperty("focused", focused);
            searchShell->style()->unpolish(searchShell);
            searchShell->style()->polish(searchShell);
            searchShell->update();
        });

        table->setColumnCount(18);
        table->setHorizontalHeaderLabels({"Name", "Platform", "Difficulty", "Tags", "Status", "Time", "Date", "Rating", "Notes",
                                          "Id", "URL", "Solution", "Due", "Interval", "Stability", "Reps",
                                          "Time complexity", "Space complexity"});
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(false);
        table->setShowGrid(false);
        table->setWordWrap(false);
        table->setFrameShape(QFrame::NoFrame);
        table->setMouseTracking(true);
        table->viewport()->setMouseTracking(true);
        table->viewport()->setAttribute(Qt::WA_Hover, true);
        table->setTextElideMode(Qt::ElideRight);
        table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        table->setCornerButtonEnabled(false);
        table->verticalHeader()->setVisible(false);
        table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        table->verticalHeader()->setDefaultSectionSize(52);

        libraryHeader = new LibraryHeader(table);
        libraryHeader->setDarkTheme(mw->isDarkMode);
        table->setHorizontalHeader(libraryHeader);
        auto *header = libraryHeader;
        header->setSectionResizeMode(0, QHeaderView::Stretch);
        header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(3, QHeaderView::Stretch);
        header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(6, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(7, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(12, QHeaderView::ResizeToContents);
        table->setColumnHidden(8, true);
        table->setColumnHidden(9, true);
        table->setColumnHidden(10, true);
        table->setColumnHidden(11, true);
        table->setColumnHidden(13, true);
        table->setColumnHidden(14, true);
        table->setColumnHidden(15, true);
        table->setColumnHidden(16, true);
        table->setColumnHidden(17, true);
        table->viewport()->setContextMenuPolicy(Qt::CustomContextMenu);
        table->setProperty("hoverRow", -1);
        auto *hoverWatcher = new RowHoverWatcher(table);
        table->viewport()->installEventFilter(hoverWatcher);

        badges = new ProblemBadgeDelegate(table);
        badges->setDarkTheme(mw->isDarkMode);
        table->setItemDelegate(badges);

        QObject::connect(table->viewport(), &QWidget::customContextMenuRequested, mw, [mw](const QPoint &pos) {
            const int row = mw->ui->table->rowAt(pos.y());
            if (row < 0)
                return;
            mw->ui->table->selectRow(row);
            const auto cell = [&](int column) {
                const auto *item = mw->ui->table->item(row, column);
                return item ? item->text().trimmed() : QString();
            };
            const QString today = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
            const QString due = cell(12);
            QMenu menu(mw);
            QAction *openUrl = menu.addAction(QStringLiteral("Open problem"));
            openUrl->setEnabled(!cell(10).isEmpty());
            QAction *openSolution = menu.addAction(QStringLiteral("Open solution"));
            openSolution->setEnabled(!cell(11).isEmpty());
            QAction *mark = menu.addAction(QStringLiteral("Mark reviewed"));
            mark->setEnabled(!due.isEmpty() && due <= today);
            menu.addSeparator();
            QAction *edit = menu.addAction(QStringLiteral("Edit"));
            QAction *chosen = menu.exec(mw->ui->table->viewport()->mapToGlobal(pos));
            if (chosen == openUrl)
                mw->openSelectedProblemUrl();
            else if (chosen == openSolution)
                mw->openSelectedSolution();
            else if (chosen == mark)
                mw->markSelectedReviewed();
            else if (chosen == edit)
                mw->editSelectedProblem();
        });
        QObject::connect(table, &QTableWidget::cellDoubleClicked, mw, [mw](int row, int) {
            if (row < 0)
                return;
            mw->ui->table->selectRow(row);
            const auto *urlItem = mw->ui->table->item(row, 10);
            const QString url = urlItem ? urlItem->text().trimmed() : QString();
            if (!url.isEmpty())
                mw->openSelectedProblemUrl();
            else
                mw->openSelectedSolution();
        });

        detailsContainer = makePanel("detailsContainer", mw);
        auto *detailsLayout = new QVBoxLayout(detailsContainer);
        detailsLayout->setContentsMargins(12, 8, 12, 10);
        detailsLayout->setSpacing(8);

        auto *detailsHead = new QHBoxLayout;
        detailsHead->setSpacing(8);
        detailsNameLabel = new QLabel(detailsContainer);
        detailsNameLabel->setTextFormat(Qt::RichText);
        detailsNameLabel->setWordWrap(true);
        detailsToggle = new QToolButton(detailsContainer);
        detailsToggle->setObjectName("detailsToggle");
        detailsToggle->setCheckable(true);
        detailsToggle->setChecked(true);
        detailsToggle->setArrowType(Qt::DownArrow);
        detailsToggle->setAutoRaise(true);
        detailsToggle->setCursor(Qt::PointingHandCursor);
        detailsToggle->setToolTip("Hide details");
        detailsHead->addWidget(detailsNameLabel, 1);
        detailsHead->addWidget(detailsToggle, 0, Qt::AlignTop);

        detailsBody = new QWidget(detailsContainer);
        auto *bodyLayout = new QVBoxLayout(detailsBody);
        bodyLayout->setContentsMargins(0, 0, 0, 0);
        bodyLayout->setSpacing(8);
        dueLabel = new QLabel(detailsBody);
        dueLabel->setObjectName("pageHint");
        dueLabel->setWordWrap(true);
        detailsMeta = new QLabel(detailsBody);
        detailsMeta->setObjectName("pageHint");
        detailsMeta->setWordWrap(true);
        detailsPath = new QLabel(detailsBody);
        detailsPath->setObjectName("pageHint");
        detailsPath->setWordWrap(true);
        detailsPath->setTextInteractionFlags(Qt::TextSelectableByMouse);
        detailsNotesText = new QTextEdit(detailsBody);
        detailsNotesText->setObjectName("detailsNotes");
        detailsNotesText->setReadOnly(true);
        detailsNotesText->setFrameShape(QFrame::NoFrame);
        detailsNotesText->setMaximumHeight(85);
        detailsNotesText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        detailsNotesText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        auto *reviewRow = new QHBoxLayout;
        reviewRow->setSpacing(8);
        const auto addReview = [&](const QString &text, const QString &name, int rating) {
            auto *button = makeButton(text, name, detailsBody);
            button->setMinimumHeight(32);
            reviewRow->addWidget(button);
            QObject::connect(button, &QPushButton::clicked, mw, [mw, rating]() { mw->reviewSelected(rating); });
            return button;
        };
        addReview("Again  1", "reviewAgain", 1);
        addReview("Hard  2", "reviewHard", 2);
        addReview("Good  3", "reviewGood", 3);
        addReview("Easy  4", "reviewEasy", 4);

        auto *linkRow = new QHBoxLayout;
        linkRow->setSpacing(8);
        openUrlButton = makeButton("Open problem", "ghostButton", detailsBody);
        openSolutionButton = makeButton("Open solution", "ghostButton", detailsBody);
        markReviewedButton = makeButton("Mark reviewed", "ghostButton", detailsBody);
        auto *editButton = makeButton("Edit", "ghostButton", detailsBody);
        linkRow->addWidget(openUrlButton);
        linkRow->addWidget(openSolutionButton);
        linkRow->addWidget(markReviewedButton);
        linkRow->addWidget(editButton);
        QObject::connect(openUrlButton, &QPushButton::clicked, mw, &MainWindow::openSelectedProblemUrl);
        QObject::connect(openSolutionButton, &QPushButton::clicked, mw, &MainWindow::openSelectedSolution);
        QObject::connect(markReviewedButton, &QPushButton::clicked, mw, &MainWindow::markSelectedReviewed);
        QObject::connect(editButton, &QPushButton::clicked, mw, &MainWindow::editSelectedProblem);
        QObject::connect(detailsToggle, &QToolButton::toggled, detailsBody, [this](bool open) {
            detailsBody->setVisible(open);
            detailsToggle->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
            detailsToggle->setToolTip(open ? QStringLiteral("Hide details") : QStringLiteral("Show details"));
        });

        for (int key : {Qt::Key_1, Qt::Key_2, Qt::Key_3, Qt::Key_4})
        {
            for (QWidget *scope : {static_cast<QWidget *>(table), static_cast<QWidget *>(detailsContainer)})
            {
                auto *shortcut = new QShortcut(QKeySequence(key), scope);
                shortcut->setContext(Qt::WidgetWithChildrenShortcut);
                QObject::connect(shortcut, &QShortcut::activated, mw, [mw, key]() {
                    mw->reviewSelected(key - Qt::Key_0);
                });
            }
        }

        bodyLayout->addWidget(dueLabel);
        bodyLayout->addWidget(detailsMeta);
        bodyLayout->addWidget(detailsPath);
        bodyLayout->addWidget(detailsNotesText);
        bodyLayout->addLayout(reviewRow);
        bodyLayout->addLayout(linkRow);
        detailsLayout->addLayout(detailsHead);
        detailsLayout->addWidget(detailsBody);
        detailsContainer->setVisible(false);

        QObject::connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, mw, [this, mw]() {
            const auto rows = table->selectionModel()->selectedRows();
            if (rows.isEmpty())
            {
                detailsContainer->setVisible(false);
                return;
            }
            const int row = rows.first().row();
            const auto cell = [this, row](int column) {
                const auto *item = table->item(row, column);
                return item ? item->text() : QString();
            };
            const QString mainColor = mw->isDarkMode ? "#F5F5F7" : "#1C1C1E";
            const QString subColor = mw->isDarkMode ? "#98989D" : "#6E6E73";
            QString tags = cell(3).toHtmlEscaped();
            tags.replace("|", " · ");
            QString html = QString("<span style='color:%1; font-weight:600; font-size:15px;'>%2</span>")
                               .arg(mainColor, cell(0).toHtmlEscaped());
            if (!tags.isEmpty())
                html += QString("<span style='color:%1; font-weight:400; font-size:12px;'>&nbsp;&nbsp;%2</span>").arg(subColor, tags);
            const QString due = cell(12);
            detailsNameLabel->setText(html);
            const QString notes = cell(8);
            detailsNotesText->setPlainText(notes.isEmpty() ? QStringLiteral("No notes for this problem.") : notes);
            if (dueLabel)
            {
                dueLabel->setText(due.isEmpty()
                                      ? QString()
                                      : QString("Due %1  ·  interval %2 days  ·  %3 reviews")
                                            .arg(due, cell(13).isEmpty() ? QString("0") : cell(13),
                                                 cell(15).isEmpty() ? QString("0") : cell(15)));
            }
            const QString timeComplexity = cell(16).trimmed();
            const QString spaceComplexity = cell(17).trimmed();
            QStringList complexity;
            if (!timeComplexity.isEmpty())
                complexity << QStringLiteral("Time %1").arg(timeComplexity);
            if (!spaceComplexity.isEmpty())
                complexity << QStringLiteral("Space %1").arg(spaceComplexity);
            if (detailsMeta)
                detailsMeta->setText(complexity.isEmpty() ? QStringLiteral("Complexity not set") : complexity.join(QStringLiteral("   ·   ")));
            const QString solution = cell(11).trimmed();
            if (detailsPath)
            {
                detailsPath->setText(solution.isEmpty() ? QStringLiteral("No solution file") : QStringLiteral("File  %1").arg(solution));
                detailsPath->setToolTip(solution);
            }
            if (openUrlButton)
                openUrlButton->setEnabled(!cell(10).trimmed().isEmpty());
            if (openSolutionButton)
                openSolutionButton->setEnabled(!solution.isEmpty());
            if (markReviewedButton)
            {
                const QString today = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
                markReviewedButton->setEnabled(!due.isEmpty() && due <= today);
            }
            if (detailsToggle && !detailsToggle->isChecked())
                detailsToggle->setChecked(true);
            detailsContainer->setVisible(true);
        });

        statsLabel->setObjectName("statsPanel");
        statsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        statsLabel->setWordWrap(true);

        difficultyChart = new DifficultyDonut(mw);
        difficultyChart->setDarkTheme(mw->isDarkMode);

        heatmap = new HeatmapWidget(mw);

        auto *sidebar = makePanel("sidebar", mw);
        sidebar->setFixedWidth(324);
        auto *sidebarLayout = new QVBoxLayout(sidebar);
        sidebarLayout->setContentsMargins(0, 0, 0, 0);
        sidebarLayout->setSpacing(0);

        auto *sidebarHeader = new QWidget(sidebar);
        sidebarHeader->setObjectName("sidebarHeader");
        sidebarHeader->setAttribute(Qt::WA_StyledBackground, true);
        auto *headerLayout = new QVBoxLayout(sidebarHeader);
        headerLayout->setContentsMargins(16, 16, 16, 12);
        headerLayout->setSpacing(10);

        auto *sidebarScroll = new QScrollArea(sidebar);
        sidebarScroll->setObjectName("sidebarScroll");
        sidebarScroll->setWidgetResizable(true);
        sidebarScroll->setFrameShape(QFrame::NoFrame);
        sidebarScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sidebarScroll->setAttribute(Qt::WA_StyledBackground, true);
        sidebarScroll->viewport()->setAutoFillBackground(false);

        auto *sidebarBody = new QWidget;
        sidebarBody->setObjectName("sidebarBody");
        sidebarBody->setAttribute(Qt::WA_StyledBackground, true);
        sidebarBody->setAutoFillBackground(false);
        auto *body = new QVBoxLayout(sidebarBody);
        body->setContentsMargins(16, 12, 16, 18);
        body->setSpacing(8);
        sidebarScroll->setWidget(sidebarBody);
        sidebarLayout->addWidget(sidebarHeader);
        sidebarLayout->addWidget(sidebarScroll, 1);

        auto *mark = new QLabel("A", sidebarHeader);
        mark->setObjectName("brandMark");
        mark->setAlignment(Qt::AlignCenter);
        mark->setFixedSize(28, 28);
        auto *brandTitle = new QLabel("AlgoTrack", mw);
        brandTitle->setObjectName("appTitle");
        auto *brandSubtitle = new QLabel("Practice log", mw);
        brandSubtitle->setObjectName("appSubtitle");
        auto *brandText = new QVBoxLayout;
        brandText->setSpacing(0);
        brandText->addWidget(brandTitle);
        brandText->addWidget(brandSubtitle);
        themeButton = new QPushButton(themeAction->text(), mw);
        themeButton->setObjectName("themeButton");
        themeButton->setCursor(Qt::PointingHandCursor);
        themeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        QObject::connect(themeButton, &QPushButton::clicked, themeAction, &QAction::trigger);
        auto *brandRow = new QHBoxLayout;
        brandRow->setSpacing(10);
        brandRow->addWidget(mark);
        brandRow->addLayout(brandText, 1);
        brandRow->addWidget(themeButton);

        auto *addButton = makeButton("Add problem", "primaryButton", sidebarHeader);
        addButton->setMinimumHeight(40);
        headerLayout->addLayout(brandRow);
        headerLayout->addWidget(addButton);
        auto *openButton = makeButton("Open", "ghostButton", mw);
        auto *saveButton = makeButton("Save", "ghostButton", mw);
        auto *deleteButton = makeButton("Delete", "dangerButton", mw);
        auto *fileRow = new QHBoxLayout;
        fileRow->setSpacing(8);
        fileRow->addWidget(openButton);
        fileRow->addWidget(saveButton);

        auto makeChip = [mw](const QString &text, bool checkable) {
            auto *button = new QPushButton(text, mw);
            button->setObjectName("chipButton");
            button->setCursor(Qt::PointingHandCursor);
            button->setCheckable(checkable);
            button->setMinimumHeight(30);
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            return button;
        };
        auto *allChip = makeChip("All", true);
        auto *easyChip = makeChip("Easy", true);
        auto *mediumChip = makeChip("Medium", true);
        auto *hardChip = makeChip("Hard", true);
        auto *solvedChip = makeChip("Solved", true);
        auto *failedChip = makeChip("Failed", true);
        auto *progressChip = makeChip("In progress", true);
        auto *sortDifficultyButton = makeChip("Difficulty", false);
        auto *sortTimeButton = makeChip("Time", false);
        auto *sortRatingButton = makeChip("Rating", false);
        allChip->setToolTip("Show the full library");
        easyChip->setToolTip("diff:easy");
        mediumChip->setToolTip("diff:medium");
        hardChip->setToolTip("diff:hard");
        solvedChip->setToolTip("status:solved");
        failedChip->setToolTip("status:failed");
        progressChip->setToolTip("status:inprogress");
        easyChip->setProperty("tone", "easy");
        mediumChip->setProperty("tone", "medium");
        hardChip->setProperty("tone", "hard");
        solvedChip->setProperty("tone", "solved");
        failedChip->setProperty("tone", "failed");
        progressChip->setProperty("tone", "progress");

        filterGroup = new QButtonGroup(mw);
        filterGroup->setExclusive(true);
        for (auto *chip : {allChip, easyChip, mediumChip, hardChip, solvedChip, failedChip, progressChip})
            filterGroup->addButton(chip);
        allChip->setChecked(true);

        auto *filters = new QGridLayout;
        filters->setHorizontalSpacing(8);
        filters->setVerticalSpacing(8);
        filters->addWidget(allChip, 0, 0);
        filters->addWidget(easyChip, 0, 1);
        filters->addWidget(mediumChip, 0, 2);
        filters->addWidget(hardChip, 0, 3);
        filters->addWidget(solvedChip, 1, 0);
        filters->addWidget(failedChip, 1, 1);
        filters->addWidget(progressChip, 1, 2, 1, 2);
        for (int column = 0; column < 4; ++column)
            filters->setColumnStretch(column, 1);

        auto *sortRow = new QHBoxLayout;
        sortRow->setSpacing(8);
        sortRow->addWidget(sortDifficultyButton);
        sortRow->addWidget(sortTimeButton);
        sortRow->addWidget(sortRatingButton);

        auto *metrics = new QGridLayout;
        metrics->setHorizontalSpacing(8);
        metrics->setVerticalSpacing(8);
        metrics->addWidget(makeMetricCard("Total", totalValue, mw), 0, 0);
        metrics->addWidget(makeMetricCard("Solved", solvedValue, mw), 0, 1);
        metrics->addWidget(makeMetricCard("Avg rating", ratingValue, mw), 1, 0);
        metrics->addWidget(makeMetricCard("Time", timeValue, mw), 1, 1);
        metrics->setColumnStretch(0, 1);
        metrics->setColumnStretch(1, 1);

        auto section = [](const QString &text, QWidget *parent) {
            auto *label = new QLabel(text, parent);
            label->setObjectName("sectionLabel");
            return label;
        };

        body->addWidget(section("Library", mw));
        body->addLayout(fileRow);
        body->addWidget(deleteButton);
        dueButton = makeButton("Due today", "chipButton", mw);
        dueButton->setCheckable(true);
        dueButton->setToolTip("Problems whose next review is today or earlier");
        body->addWidget(section("Quick filters", mw));
        body->addLayout(filters);
        body->addWidget(section("Review", mw));
        body->addWidget(dueButton);
        body->addWidget(section("Sort", mw));
        body->addLayout(sortRow);
        body->addWidget(section("Overview", mw));
        body->addLayout(metrics);
        body->addWidget(section("Breakdown", mw));
        body->addWidget(statsLabel);
        body->addWidget(difficultyChart);
        body->addWidget(heatmap);
        body->addStretch();

        auto *tablePanel = makePanel("tablePanel", mw);
        auto *tableLayout = new QVBoxLayout(tablePanel);
        tableLayout->setContentsMargins(8, 8, 8, 8);
        tableLayout->setSpacing(0);
        tableLayout->addWidget(table, 1);
        tableLayout->addWidget(detailsContainer);

        auto *content = new QWidget(mw);
        content->setObjectName("contentColumn");
        content->setAttribute(Qt::WA_StyledBackground, true);
        content->setAutoFillBackground(false);
        auto *contentLayout = new QVBoxLayout(content);
        contentLayout->setContentsMargins(8, 4, 4, 0);
        contentLayout->setSpacing(14);
        auto *pageTitle = new QLabel("Library", content);
        pageTitle->setObjectName("pageTitle");
        auto *pageHint = new QLabel("Press Enter to search. Try diff:hard, status:solved, platform:leetcode, or tag:graphs.", content);
        pageHint->setObjectName("pageHint");
        pageHint->setWordWrap(true);
        auto *heading = new QVBoxLayout;
        heading->setSpacing(4);
        heading->setContentsMargins(4, 2, 4, 0);
        heading->addWidget(pageTitle);
        heading->addWidget(pageHint);

        auto *timerShell = new QFrame(content);
        timerShell->setObjectName("timerShell");
        timerShell->setAttribute(Qt::WA_StyledBackground, true);
        auto *timerLayout = new QHBoxLayout(timerShell);
        timerLayout->setContentsMargins(12, 6, 8, 6);
        timerLayout->setSpacing(8);
        timerReadout = new QLabel(QStringLiteral("00:00:00"), timerShell);
        timerReadout->setObjectName("timerReadout");
        QFont timerFont = timerReadout->font();
        timerFont.setFamilies({"Cascadia Mono", "Cascadia Code", "Consolas", "SF Mono"});
        timerFont.setStyleHint(QFont::Monospace);
        timerFont.setPixelSize(16);
        timerFont.setWeight(QFont::Bold);
        timerReadout->setFont(timerFont);
        timerToggle = new QPushButton(QStringLiteral("Start"), timerShell);
        timerToggle->setObjectName("timerButton");
        timerToggle->setCursor(Qt::PointingHandCursor);
        timerReset = new QPushButton(QStringLiteral("Reset"), timerShell);
        timerReset->setObjectName("timerButton");
        timerReset->setCursor(Qt::PointingHandCursor);
        timerLayout->addWidget(timerReadout);
        timerLayout->addWidget(timerToggle);
        timerLayout->addWidget(timerReset);

        auto *headingRow = new QHBoxLayout;
        headingRow->setSpacing(12);
        headingRow->addLayout(heading, 1);
        headingRow->addWidget(timerShell, 0, Qt::AlignTop);
        contentLayout->addLayout(headingRow);
        contentLayout->addWidget(searchShell);
        contentLayout->addWidget(tablePanel, 1);

        auto *root = new QWidget(mw);
        root->setObjectName("appRoot");
        root->setAttribute(Qt::WA_StyledBackground, true);
        auto *rootLayout = new QHBoxLayout(root);
        rootLayout->setContentsMargins(16, 12, 16, 12);
        rootLayout->setSpacing(14);
        rootLayout->addWidget(sidebar);
        rootLayout->addWidget(content, 1);
        mw->setCentralWidget(root);
        mw->statusBar()->addWidget(statusLabel, 1);

        auto *findShortcut = new QShortcut(QKeySequence::Find, mw);
        findShortcut->setContext(Qt::WindowShortcut);
        QObject::connect(findShortcut, &QShortcut::activated, searchEdit, [this]() {
            searchEdit->setFocus(Qt::ShortcutFocusReason);
            searchEdit->selectAll();
        });

        auto *newShortcut = new QShortcut(QKeySequence::New, mw);
        newShortcut->setContext(Qt::WindowShortcut);
        QObject::connect(newShortcut, &QShortcut::activated, mw, &MainWindow::addProblem);

        auto *timerShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space), mw);
        timerShortcut->setContext(Qt::WindowShortcut);
        QObject::connect(timerShortcut, &QShortcut::activated, mw, &MainWindow::togglePracticeTimer);

        auto *clearShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), mw);
        clearShortcut->setContext(Qt::WindowShortcut);
        QObject::connect(clearShortcut, &QShortcut::activated, mw, &MainWindow::clearSearchAndFilters);

        auto syncChips = [=](const QString &text) {
            const QString query = text.trimmed().toLower();
            if (mw->showingDue && query.isEmpty())
            {
                filterGroup->setExclusive(false);
                for (QAbstractButton *button : filterGroup->buttons())
                    button->setChecked(false);
                filterGroup->setExclusive(true);
                return;
            }
            if (!query.isEmpty() && mw->showingDue)
            {
                mw->showingDue = false;
                if (dueButton)
                    dueButton->setChecked(false);
            }
            QAbstractButton *match = nullptr;
            if (query.isEmpty())
                match = allChip;
            else if (query == QLatin1String("diff:easy"))
                match = easyChip;
            else if (query == QLatin1String("diff:medium"))
                match = mediumChip;
            else if (query == QLatin1String("diff:hard"))
                match = hardChip;
            else if (query == QLatin1String("status:solved"))
                match = solvedChip;
            else if (query == QLatin1String("status:failed"))
                match = failedChip;
            else if (query == QLatin1String("status:inprogress") || query == QLatin1String("status:in_progress") || query == QLatin1String("status:in-progress"))
                match = progressChip;

            const auto buttons = filterGroup->buttons();
            QAbstractButton *current = nullptr;
            for (QAbstractButton *button : buttons)
            {
                if (button->isChecked())
                {
                    current = button;
                    break;
                }
            }
            if (current == match)
                return;

            filterGroup->setExclusive(false);
            for (QAbstractButton *button : buttons)
                button->setChecked(button == match);
            filterGroup->setExclusive(true);
        };

        QObject::connect(addButton, &QPushButton::clicked, mw, &MainWindow::addProblem);
        QObject::connect(timerToggle, &QPushButton::clicked, mw, &MainWindow::togglePracticeTimer);
        QObject::connect(timerReset, &QPushButton::clicked, mw, &MainWindow::resetPracticeTimer);
        QObject::connect(openButton, &QPushButton::clicked, mw, &MainWindow::loadFile);
        QObject::connect(saveButton, &QPushButton::clicked, mw, &MainWindow::saveFile);
        QObject::connect(deleteButton, &QPushButton::clicked, mw, &MainWindow::deleteSelectedProblem);
        QObject::connect(dueButton, &QPushButton::clicked, mw, &MainWindow::showDueToday);
        QObject::connect(searchGlyph, &QAction::triggered, mw, &MainWindow::applySearch);
        QObject::connect(clearGlyph, &QAction::triggered, mw, [mw]() {
            mw->ui->searchEdit->clear();
            mw->applySearch();
        });
        QObject::connect(searchEdit, &QLineEdit::returnPressed, mw, &MainWindow::applySearch);
        QObject::connect(searchEdit, &QLineEdit::textChanged, mw, [syncChips, this](const QString &text) {
            clearGlyph->setVisible(!text.trimmed().isEmpty());
            syncChips(text);
        });
        QObject::connect(allChip, &QPushButton::clicked, mw, [mw]() {
            mw->ui->searchEdit->setText(QString());
            mw->applySearch();
        });
        QObject::connect(easyChip, &QPushButton::clicked, mw, [mw]() {
            mw->ui->searchEdit->setText("diff:easy");
            mw->applySearch();
        });
        QObject::connect(mediumChip, &QPushButton::clicked, mw, [mw]() {
            mw->ui->searchEdit->setText("diff:medium");
            mw->applySearch();
        });
        QObject::connect(hardChip, &QPushButton::clicked, mw, [mw]() {
            mw->ui->searchEdit->setText("diff:hard");
            mw->applySearch();
        });
        QObject::connect(solvedChip, &QPushButton::clicked, mw, [mw]() {
            mw->ui->searchEdit->setText("status:solved");
            mw->applySearch();
        });
        QObject::connect(failedChip, &QPushButton::clicked, mw, [mw]() {
            mw->ui->searchEdit->setText("status:failed");
            mw->applySearch();
        });
        QObject::connect(progressChip, &QPushButton::clicked, mw, [mw]() {
            mw->ui->searchEdit->setText("status:inprogress");
            mw->applySearch();
        });
        auto sortFullLibrary = [mw](void (MainWindow::*slot)()) {
            if (!mw->ui->searchEdit->text().isEmpty())
                mw->ui->searchEdit->clear();
            (mw->*slot)();
        };
        QObject::connect(sortDifficultyButton, &QPushButton::clicked, mw, [sortFullLibrary]() {
            sortFullLibrary(&MainWindow::sortByDifficulty);
        });
        QObject::connect(sortTimeButton, &QPushButton::clicked, mw, [sortFullLibrary]() {
            sortFullLibrary(&MainWindow::sortByTime);
        });
        QObject::connect(sortRatingButton, &QPushButton::clicked, mw, [sortFullLibrary]() {
            sortFullLibrary(&MainWindow::sortByRating);
        });
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(std::make_unique<MainWindowImpl>()),
      cliProcess(new QProcess(this)),
      practiceTimer(new QTimer(this))
{
    practiceTimer->setInterval(200);
    connect(practiceTimer, &QTimer::timeout, this, &MainWindow::updatePracticeReadout);
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
    if (ui->badges)
        ui->badges->setDarkTheme(isDarkMode);
    if (ui->libraryHeader)
        ui->libraryHeader->setDarkTheme(isDarkMode);
    if (ui->difficultyChart)
        ui->difficultyChart->setDarkTheme(isDarkMode);
    if (ui->heatmap)
        ui->heatmap->setDarkTheme(isDarkMode);

    qApp->setPalette(glassPalette(isDarkMode));

    const QString qssResourcePath = isDarkMode ? ":/styles/dark.qss" : ":/styles/light.qss";
    QFile file(qssResourcePath);
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        qApp->setStyleSheet(QString::fromUtf8(file.readAll()));
        file.close();
    }

    const QColor glyph = isDarkMode ? QColor(235, 235, 245, 170) : QColor(60, 60, 67, 150);
    if (ui->searchGlyph)
        ui->searchGlyph->setIcon(makeGlyphIcon(QStringLiteral("search"), glyph));
    if (ui->clearGlyph)
        ui->clearGlyph->setIcon(makeGlyphIcon(QStringLiteral("clear"), glyph));
    if (ui->badges)
    {
        ui->badges->setDarkTheme(isDarkMode);
        ui->table->viewport()->update();
    }

    ui->themeAction->setText(isDarkMode ? "Light Mode" : "Dark Mode");
    if (ui->themeButton)
        ui->themeButton->setText(ui->themeAction->text());

    if (ui->difficultyChart)
        ui->difficultyChart->update();
    if (ui->heatmap)
        ui->heatmap->update();
    if (ui->libraryHeader)
        ui->libraryHeader->viewport()->update();
    if (ui->table)
        ui->table->viewport()->update();
    if (centralWidget())
        centralWidget()->update();
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
                              fitBadgeColumns(ui->table);
                          },
                          [this](const IPC::StartStats &)
                          {
                              readingStats = true;
                              const QString textCol = isDarkMode ? "#D1D1D6" : "#3A3A3C";
                              statsHtmlBuffer = QString("<div style='font-size:13px; color:%1; line-height:1.55;'>").arg(textCol);
                              statEasy = statMedium = statHard = statSolved = statTotal = 0;
                              statDates.clear();
                          },
                          [this](const IPC::EndStats &)
                          {
                              readingStats = false;
                              statsHtmlBuffer += "</div>";
                              ui->statsLabel->setText(statsHtmlBuffer);

                              if (ui->difficultyChart)
                                  ui->difficultyChart->setCounts(statEasy, statMedium, statHard, statSolved, statTotal);
                              ui->heatmap->setData(statDates);
                          },
                          [this](const IPC::Success &)
                          {
                              if (resetTimerAfterAdd)
                              {
                                  resetTimerAfterAdd = false;
                                  resetPracticeTimer();
                              }
                              refreshLibrary();
                          },
                          [this](const IPC::Error &m)
                          {
                              resetTimerAfterAdd = false;
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
    ui->table->clearContents();
    ui->table->setRowCount(0);
    addCsvRowToTable(csv);
    fitBadgeColumns(ui->table);
    ui->table->viewport()->update();
    ui->table->selectRow(0);
}

void MainWindow::addCsvRowToTable(const QString &csv)
{
    int row = ui->table->rowCount();
    ui->table->insertRow(row);
    const std::vector<std::string> fields = parseCSVLine(csv.toStdString());
    for (int i = 0; i < static_cast<int>(fields.size()) && i < ui->table->columnCount(); ++i)
    {
        QString cleanField = QString::fromStdString(fields[static_cast<size_t>(i)]).trimmed();
        if (i == 7)
        {
            bool ok = false;
            const double val = cleanField.toDouble(&ok);
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
    {
        ui->totalValue->setText(val);
        statTotal = val.toInt();
    }
    else if (key == "solved")
    {
        ui->solvedValue->setText(val);
        statSolved = val.toInt();
    }
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
        QString c = isDarkMode ? "#FCA5A5" : "#9F1239";
        statsHtmlBuffer += QString("<div style='margin-bottom:6px;'><b>Failed:</b> <span style='color:%1; font-weight:600;'>%2</span></div>").arg(c, val);
    }
    else if (key == "progress")
    {
        QString c = isDarkMode ? "#F97316" : "#9A3412";
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
    else if (key == "due_today" && ui->dueButton)
        ui->dueButton->setText(val == "0" ? QString("Due today") : QString("Due today  %1").arg(val));
    else if (key == "most_used_tag")
    {
        if (isDarkMode)
        {
            statsHtmlBuffer += QString("<div style='margin-top:16px; border-top:1px solid #3A3A3C; padding-top:8px;'><b style='color:#F5F5F7;'>Most used tag:</b></div><div style='display:inline-block; background:#2C2C2E; color:#64D2FF; padding:3px 8px; border-radius:9px; font-weight:600; margin-top:4px;'>%1</div>").arg(val);
        }
        else
        {
            statsHtmlBuffer += QString("<div style='margin-top:16px; border-top:1px solid #E5E5EA; padding-top:8px;'><b style='color:#1C1C1E;'>Most used tag:</b></div><div style='display:inline-block; background:#E8F1FF; color:#007AFF; padding:3px 8px; border-radius:9px; font-weight:600; margin-top:4px;'>%1</div>").arg(val);
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
    auto dialog = AddProblemDialog::create(this, practiceMinutesForDialog());
    if (dialog->exec() != QDialog::Accepted)
        return;
    if (cliProcess->state() == QProcess::Running)
        resetTimerAfterAdd = true;
    Problem p = dialog->problem();
    sendCommandToCli("ADD " + QString::fromStdString(p.toCSV()));
}

void MainWindow::clearSearchAndFilters()
{
    showingDue = false;
    if (ui->dueButton)
        ui->dueButton->setChecked(false);
    if (!ui->searchEdit->text().isEmpty())
        ui->searchEdit->clear();
    applySearch();
}

void MainWindow::deleteSelectedProblem()
{
    int row = ui->table->currentRow();
    const QString id = (row >= 0 && ui->table->item(row, 9)) ? ui->table->item(row, 9)->text().trimmed() : QString();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "Nicio selecție", "Selectează o problemă mai întâi.");
        return;
    }
    if (QMessageBox::question(this, "Șterge problemă", "Ești sigur că vrei să ștergi problema?") != QMessageBox::Yes)
        return;
    sendCommandToCli("DELETE_ID " + id);
}

void MainWindow::editSelectedProblem()
{
    const int row = ui->table->currentRow();
    if (row < 0 || !ui->table->item(row, 9))
    {
        QMessageBox::information(this, "Nicio selecție", "Selectează o problemă mai întâi.");
        return;
    }
    const auto cell = [this, row](int column) {
        const auto *item = ui->table->item(row, column);
        return item ? item->text().trimmed().toStdString() : std::string();
    };
    try
    {
        std::vector<std::string> tags;
        const QStringList rawTags = QString::fromStdString(cell(3)).split('|', Qt::SkipEmptyParts);
        for (const QString &tag : rawTags)
        {
            if (!tag.trimmed().isEmpty())
                tags.push_back(tag.trimmed().toStdString());
        }
        Problem current(cell(0), cell(1), parseDifficulty(cell(2)), tags, parseStatus(cell(4)),
                        std::stoi(cell(5)), cell(6), parseCsvDouble(cell(7)), cell(8),
                        cell(10), cell(11), cell(9), cell(12),
                        std::stoi(cell(13).empty() ? "0" : cell(13)),
                        parseCsvDouble(cell(14).empty() ? "1" : cell(14)),
                        std::stoi(cell(15).empty() ? "0" : cell(15)),
                        cell(16), cell(17));
        auto dialog = AddProblemDialog::edit(current, this);
        if (dialog->exec() != QDialog::Accepted)
            return;
        const Problem updated = dialog->problem();
        sendCommandToCli("EDIT_ID " + QString::fromStdString(updated.getId()) + " " +
                          QString::fromStdString(updated.toCSV()));
    }
    catch (const std::exception &error)
    {
        QMessageBox::warning(this, "Invalid problem", error.what());
    }
}

void MainWindow::showDueToday()
{
    showingDue = true;
    if (ui->dueButton)
        ui->dueButton->setChecked(true);
    if (ui->filterGroup)
    {
        ui->filterGroup->setExclusive(false);
        for (QAbstractButton *button : ui->filterGroup->buttons())
            button->setChecked(false);
        ui->filterGroup->setExclusive(true);
    }
    ui->searchEdit->clear();
    sendCommandToCli("DUE");
}

void MainWindow::reviewSelected(int rating)
{
    const int row = ui->table->currentRow();
    const QString id = (row >= 0 && ui->table->item(row, 9)) ? ui->table->item(row, 9)->text().trimmed() : QString();
    if (id.isEmpty() || rating < 1 || rating > 4)
        return;
    sendCommandToCli(QString("REVIEW %1 %2").arg(id).arg(rating));
}

void MainWindow::refreshLibrary()
{
    if (showingDue)
        sendCommandToCli("DUE");
    else if (!ui->searchEdit->text().trimmed().isEmpty())
        sendCommandToCli("SEARCH " + ui->searchEdit->text().trimmed());
    else
        sendCommandToCli("LIST");
    sendCommandToCli("STATS");
}

void MainWindow::applySearch()
{
    showingDue = false;
    if (ui->dueButton)
        ui->dueButton->setChecked(false);
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
    showingDue = false;
    if (ui->dueButton)
        ui->dueButton->setChecked(false);
    sendCommandToCli("SORT_DIFFICULTY");
}
void MainWindow::sortByTime()
{
    showingDue = false;
    if (ui->dueButton)
        ui->dueButton->setChecked(false);
    sendCommandToCli("SORT_TIME");
}
void MainWindow::sortByRating()
{
    showingDue = false;
    if (ui->dueButton)
        ui->dueButton->setChecked(false);
    sendCommandToCli("SORT_RATING");
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
void MainWindow::exportMarkdown()
{
    QString path = QFileDialog::getSaveFileName(this, "Export Markdown", QString(), "Markdown (*.md);;All files (*.*)");
    if (!path.isEmpty())
    {
        if (!path.endsWith(".md", Qt::CaseInsensitive))
            path += ".md";
        sendCommandToCli("EXPORT_MD " + path);
    }
}

qint64 MainWindow::currentPracticeMs() const
{
    qint64 elapsed = practiceElapsedMs;
    if (practiceRunning)
        elapsed += QDateTime::currentMSecsSinceEpoch() - practiceAnchorMs;
    return elapsed < 0 ? 0 : elapsed;
}

int MainWindow::practiceMinutesForDialog() const
{
    const qint64 elapsed = currentPracticeMs();
    if (elapsed <= 0)
        return -1;
    const qint64 seconds = (elapsed + 999) / 1000;
    return qMax(1, static_cast<int>((seconds + 59) / 60));
}

void MainWindow::updatePracticeReadout()
{
    if (!ui || !ui->timerReadout)
        return;
    const qint64 totalSeconds = currentPracticeMs() / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;
    ui->timerReadout->setText(QStringLiteral("%1:%2:%3")
                                  .arg(hours, 2, 10, QLatin1Char('0'))
                                  .arg(minutes, 2, 10, QLatin1Char('0'))
                                  .arg(seconds, 2, 10, QLatin1Char('0')));
}

void MainWindow::togglePracticeTimer()
{
    if (!practiceTimer)
        return;
    if (practiceRunning)
    {
        practiceElapsedMs = currentPracticeMs();
        practiceRunning = false;
        practiceTimer->stop();
        if (ui->timerToggle)
            ui->timerToggle->setText(QStringLiteral("Start"));
    }
    else
    {
        practiceAnchorMs = QDateTime::currentMSecsSinceEpoch();
        practiceRunning = true;
        practiceTimer->start();
        if (ui->timerToggle)
            ui->timerToggle->setText(QStringLiteral("Pause"));
    }
    updatePracticeReadout();
}

void MainWindow::resetPracticeTimer()
{
    practiceRunning = false;
    practiceElapsedMs = 0;
    if (practiceTimer)
        practiceTimer->stop();
    if (ui->timerToggle)
        ui->timerToggle->setText(QStringLiteral("Start"));
    updatePracticeReadout();
}

void MainWindow::openSelectedProblemUrl()
{
    const int row = ui->table->currentRow();
    if (row < 0 || !ui->table->item(row, 10))
        return;
    const QString url = ui->table->item(row, 10)->text().trimmed();
    if (!url.isEmpty())
        QDesktopServices::openUrl(QUrl::fromUserInput(url));
}

void MainWindow::openSelectedSolution()
{
    const int row = ui->table->currentRow();
    if (row < 0 || !ui->table->item(row, 11))
        return;
    const QString path = ui->table->item(row, 11)->text().trimmed();
    if (path.isEmpty())
        return;
    if (path.startsWith(QLatin1String("http://")) || path.startsWith(QLatin1String("https://")))
        QDesktopServices::openUrl(QUrl(path));
    else if (QFileInfo::exists(path))
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()));
    else
        QMessageBox::warning(this, "Solution", "The solution file could not be found.");
}

void MainWindow::markSelectedReviewed()
{
    const int row = ui->table->currentRow();
    const QString id = (row >= 0 && ui->table->item(row, 9)) ? ui->table->item(row, 9)->text().trimmed() : QString();
    if (id.isEmpty())
        return;
    const QString due = ui->table->item(row, 12) ? ui->table->item(row, 12)->text().trimmed() : QString();
    const QString today = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    if (due.isEmpty() || due > today)
    {
        ui->statusLabel->setText("This problem is not due yet.");
        return;
    }
    sendCommandToCli("MARK_REVIEWED " + id);
}
