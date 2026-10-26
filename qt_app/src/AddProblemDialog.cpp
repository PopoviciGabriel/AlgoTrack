#include "AddProblemDialog.h"
#include "ui_AddProblemDialog.h"
#include "Problem.h"
#include "Enums.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QUrl>
#include <QVBoxLayout>

#include <optional>
#include <tuple>

namespace
{
QString titleCaseSlug(QString slug)
{
    slug.replace(QRegularExpression("[-_+.]+"), " ");
    slug = slug.trimmed();
    QStringList words = slug.split(' ', Qt::SkipEmptyParts);
    for (QString &word : words)
    {
        word = word.toLower();
        if (!word.isEmpty())
            word[0] = word[0].toUpper();
    }
    return words.join(' ');
}

struct ParsedLink
{
    QString platform;
    QString name;
};

QString stripUrlNoise(QString text)
{
    text = text.trimmed();
    const int cut = text.indexOf(QRegularExpression(QStringLiteral("[?#]")));
    if (cut >= 0)
        text.truncate(cut);
    while (text.endsWith(QLatin1Char('/')))
        text.chop(1);
    return text;
}

QString stripPathSuffixes(QString path)
{
    const QStringList noise = {
        QStringLiteral("description"), QStringLiteral("solutions"), QStringLiteral("solution"),
        QStringLiteral("editorial"), QStringLiteral("submissions"), QStringLiteral("discuss"),
        QStringLiteral("comments")};
    while (path.endsWith(QLatin1Char('/')))
        path.chop(1);
    bool removed = true;
    while (removed)
    {
        removed = false;
        const int slash = path.lastIndexOf(QLatin1Char('/'));
        if (slash < 0)
            break;
        const QString tail = path.mid(slash + 1);
        for (const QString &word : noise)
        {
            if (tail.compare(word, Qt::CaseInsensitive) == 0)
            {
                path.truncate(slash);
                while (path.endsWith(QLatin1Char('/')))
                    path.chop(1);
                removed = true;
                break;
            }
        }
    }
    return path;
}

std::optional<ParsedLink> parseProblemLink(const QString &raw)
{
    QString text = stripUrlNoise(raw);
    if (text.isEmpty())
        return std::nullopt;
    if (!text.contains(QLatin1String("://")))
        text.prepend(QStringLiteral("https://"));

    const QUrl url(text);
    if (!url.isValid() || url.host().isEmpty())
        return std::nullopt;

    QString host = url.host().toLower();
    if (host.startsWith(QLatin1String("www.")))
        host.remove(0, 4);
    const QString path = stripPathSuffixes(url.path());

    auto captured = [&](const char *pattern) {
        return QRegularExpression(QString::fromLatin1(pattern), QRegularExpression::CaseInsensitiveOption).match(path);
    };

    if (host == QLatin1String("leetcode.com") || host.endsWith(QLatin1String(".leetcode.com")))
    {
        const auto match = captured("/problems/([^/]+)");
        QString name;
        if (match.hasMatch())
        {
            const QString slug = match.captured(1);
            if (slug.compare(QLatin1String("problemset"), Qt::CaseInsensitive) != 0 &&
                slug.compare(QLatin1String("description"), Qt::CaseInsensitive) != 0)
                name = titleCaseSlug(slug);
        }
        return ParsedLink{QStringLiteral("LeetCode"), name};
    }

    if (host == QLatin1String("codeforces.com") || host.endsWith(QLatin1String(".codeforces.com")))
    {
        const QRegularExpression re(
            QString::fromUtf8(R"((?:/problemset/problem/(\d+)/([^/]+))|(?:/contest/(\d+)/problem/([^/]+))|(?:/gym/(\d+)/problem/([^/]+)))"),
            QRegularExpression::CaseInsensitiveOption);
        const auto match = re.match(path);
        QString name;
        if (match.hasMatch())
        {
            QString contest = match.captured(1);
            QString index = match.captured(2);
            if (contest.isEmpty())
            {
                contest = match.captured(3);
                index = match.captured(4);
            }
            if (contest.isEmpty())
            {
                contest = match.captured(5);
                index = match.captured(6);
            }
            name = contest + index.toUpper();
        }
        return ParsedLink{QStringLiteral("Codeforces"), name};
    }

    if (host == QLatin1String("atcoder.jp"))
    {
        const auto match = captured("/contests/[^/]+/tasks/([^/]+)");
        const QString name = match.hasMatch() ? titleCaseSlug(match.captured(1)) : QString();
        return ParsedLink{QStringLiteral("AtCoder"), name};
    }

    if (host == QLatin1String("kilonova.ro"))
    {
        const auto match = captured(R"(/problems/(\d+)(?:/([^/]+))?)");
        QString name;
        if (match.hasMatch())
            name = match.captured(2).isEmpty() ? QStringLiteral("Problem %1").arg(match.captured(1))
                                               : titleCaseSlug(match.captured(2));
        return ParsedLink{QStringLiteral("Kilonova"), name};
    }

    if (host == QLatin1String("pbinfo.ro"))
    {
        const auto match = captured(R"(/probleme/(\d+)(?:/([^/]+))?)");
        QString name;
        if (match.hasMatch())
            name = match.captured(2).isEmpty() ? QStringLiteral("Problema %1").arg(match.captured(1))
                                               : titleCaseSlug(match.captured(2));
        return ParsedLink{QStringLiteral("PbInfo"), name};
    }

    return std::nullopt;
}

void setupComplexityCombo(QComboBox *combo, const QString &placeholder)
{
    combo->clear();
    combo->addItems({QStringLiteral("O(1)"), QStringLiteral("O(log N)"), QStringLiteral("O(N)"),
                     QStringLiteral("O(N log N)"), QStringLiteral("O(N^2)"), QStringLiteral("O(N^3)"),
                     QStringLiteral("O(2^N)"), QStringLiteral("O(N!)")});
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);
    combo->setCurrentIndex(-1);
    combo->setCurrentText(QString());
    if (combo->lineEdit())
        combo->lineEdit()->setPlaceholderText(placeholder);
}

void fixComboPopup(QComboBox *combo)
{
    combo->setItemDelegate(new QStyledItemDelegate(combo));
    QWidget *popup = combo->view()->window();
    popup->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    const bool dark = combo->palette().color(QPalette::Window).lightness() < 140;
    popup->setStyleSheet(dark
                             ? "background-color: #2C2C2E; color: #F5F5F7; border: 1px solid rgba(255, 255, 255, 40); border-radius: 10px;"
                             : "background-color: #FFFFFF; color: #1C1C1E; border: 1px solid rgba(0, 0, 0, 28); border-radius: 10px;");
    combo->view()->setFrameShape(QFrame::NoFrame);
}
}

class AddProblemDialogImpl final : public AddProblemDialog
{
    friend class AddProblemDialog;

public:
    Ui::AddProblemDialog ui;
    std::string keptId;
    std::string keptDue;
    int keptInterval = 0;
    double keptStability = 1.0;
    int keptReps = 0;
    int presetMinutes = -1;
    bool blockUrlParse = false;
    bool nameFromUrl = false;
    bool platformFromUrl = false;

    explicit AddProblemDialogImpl(QWidget *parent) : AddProblemDialog(parent) {}

    void loadExisting(const Problem &problem)
    {
        blockUrlParse = true;
        ui.nameEdit->setText(QString::fromStdString(problem.getName()));
        ui.platformEdit->setText(QString::fromStdString(problem.getPlatform()));
        ui.difficultyCombo->setCurrentIndex(static_cast<int>(problem.getDifficulty()));
        ui.statusCombo->setCurrentIndex(static_cast<int>(problem.getStatus()));
        QStringList tags;
        for (const auto &tag : problem.getTags())
            tags.push_back(QString::fromStdString(tag));
        ui.tagsEdit->setText(tags.join(", "));
        ui.timeSpin->setValue(problem.getTimeSpent());
        const QDate parsedDate = QDate::fromString(QString::fromStdString(problem.getDate()), "dd-MM-yyyy");
        if (parsedDate.isValid())
            ui.dateEdit->setDate(parsedDate);
        ui.ratingSpin->setValue(problem.getRating());
        ui.notesEdit->setPlainText(QString::fromStdString(problem.getNotes()));
        ui.urlEdit->setText(QString::fromStdString(problem.getProblemUrl()));
        ui.solutionEdit->setText(QString::fromStdString(problem.getSolutionPath()));
        ui.timeComplexityCombo->setCurrentText(QString::fromStdString(problem.getTimeComplexity()));
        ui.spaceComplexityCombo->setCurrentText(QString::fromStdString(problem.getSpaceComplexity()));
        keptId = problem.getId();
        keptDue = problem.getDueDate();
        keptInterval = problem.getIntervalDays();
        keptStability = problem.getStability();
        keptReps = problem.getRepetitionCount();
        nameFromUrl = false;
        platformFromUrl = false;
        ui.dialogTitle->setText("Edit problem");
        ui.dialogSubtitle->setText("The id and the review schedule stay with this problem.");
        setWindowTitle("Edit problem");
        if (auto *buttons = findChild<QDialogButtonBox *>("dialogButtons"))
            buttons->button(QDialogButtonBox::Ok)->setText("Save");
        blockUrlParse = false;
        validateInputFields();
        ui.nameEdit->setFocus();
    }

    void applyUrlAutofill()
    {
        if (blockUrlParse)
            return;
        const std::optional<ParsedLink> parsed = parseProblemLink(ui.urlEdit->text());
        if (!parsed)
            return;

        if (!parsed->platform.isEmpty() && (ui.platformEdit->text().trimmed().isEmpty() || platformFromUrl))
        {
            platformFromUrl = true;
            if (ui.platformEdit->text() != parsed->platform)
                ui.platformEdit->setText(parsed->platform);
        }
        if (!parsed->name.isEmpty() && (ui.nameEdit->text().trimmed().isEmpty() || nameFromUrl))
        {
            nameFromUrl = true;
            if (ui.nameEdit->text() != parsed->name)
                ui.nameEdit->setText(parsed->name);
        }
    }

    auto gatherData() const
    {
        QStringList rawTags = ui.tagsEdit->text().split(',', Qt::SkipEmptyParts);
        std::vector<std::string> tags;
        tags.reserve(rawTags.size());
        for (const QString &tag : rawTags)
            tags.push_back(tag.trimmed().toStdString());

        Difficulty diff = (ui.difficultyCombo->currentIndex() == 0) ? Difficulty::Easy
                          : (ui.difficultyCombo->currentIndex() == 1) ? Difficulty::Medium
                                                                      : Difficulty::Hard;

        Status stat = (ui.statusCombo->currentIndex() == 0) ? Status::Solved
                      : (ui.statusCombo->currentIndex() == 1) ? Status::Failed
                                                              : Status::InProgress;

        return std::make_tuple(
            ui.nameEdit->text().toStdString(),
            ui.platformEdit->text().toStdString(),
            diff,
            tags,
            stat,
            ui.timeSpin->value(),
            ui.dateEdit->date().toString("dd-MM-yyyy").toStdString(),
            ui.ratingSpin->value(),
            ui.notesEdit->toPlainText().toStdString(),
            ui.urlEdit->text().trimmed().toStdString(),
            ui.solutionEdit->text().trimmed().toStdString(),
            keptId,
            keptDue,
            keptInterval,
            keptStability,
            keptReps,
            ui.timeComplexityCombo->currentText().trimmed().toStdString(),
            ui.spaceComplexityCombo->currentText().trimmed().toStdString());
    }

    void validateInputFields()
    {
        auto *buttons = findChild<QDialogButtonBox *>("dialogButtons");
        if (!buttons)
            return;
        const bool hasName = !ui.nameEdit->text().trimmed().isEmpty();
        const bool hasPlatform = !ui.platformEdit->text().trimmed().isEmpty();
        const bool hasTags = !ui.tagsEdit->text().trimmed().isEmpty();
        buttons->button(QDialogButtonBox::Ok)->setEnabled(hasName && hasPlatform && hasTags);
    }
};

AddProblemDialog::AddProblemDialog(QWidget *parent) : QDialog(parent) {}

void AddProblemDialog::deleter::operator()(AddProblemDialog *ptr)
{
    delete static_cast<AddProblemDialogImpl *>(ptr);
}

namespace
{
void configureDialog(AddProblemDialogImpl *impl)
{
    impl->ui.setupUi(impl);
    if (impl->layout())
        impl->layout()->setSizeConstraint(QLayout::SetNoConstraint);
    impl->setMinimumSize(480, 420);
    impl->resize(540, 520);
    impl->setAttribute(Qt::WA_StyledBackground, true);

    if (auto *header = impl->findChild<QWidget *>("dialogHeader"))
        header->setAttribute(Qt::WA_StyledBackground, true);
    if (auto *footer = impl->findChild<QFrame *>("dialogFooter"))
        footer->setAttribute(Qt::WA_StyledBackground, true);
    if (auto *card = impl->findChild<QFrame *>("formCard"))
        card->setAttribute(Qt::WA_StyledBackground, true);

    impl->ui.formScroll->setWidgetResizable(true);
    impl->ui.formScroll->setFrameShape(QFrame::NoFrame);
    impl->ui.formScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    impl->ui.formScroll->viewport()->setAutoFillBackground(false);
    impl->ui.formScroll->viewport()->setStyleSheet(QStringLiteral("background: transparent;"));

    if (auto *grid = impl->findChild<QGridLayout *>("formGrid"))
    {
        grid->setColumnStretch(0, 2);
        grid->setColumnStretch(1, 1);
        grid->setColumnStretch(2, 1);
        grid->setHorizontalSpacing(10);
        grid->setVerticalSpacing(10);
        grid->setRowStretch(6, 1);
    }
    if (auto *session = impl->findChild<QGridLayout *>("sessionGrid"))
    {
        session->setColumnStretch(0, 1);
        session->setColumnStretch(1, 1);
    }

    impl->ui.notesEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    impl->ui.notesEdit->setMaximumHeight(85);
    impl->ui.timeSpin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    impl->ui.ratingSpin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    setupComplexityCombo(impl->ui.timeComplexityCombo, QStringLiteral("O(N)"));
    setupComplexityCombo(impl->ui.spaceComplexityCombo, QStringLiteral("O(1)"));

    QFile styleFile(":/styles/dialog.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
        impl->setStyleSheet(QLatin1String(styleFile.readAll()));

    for (QComboBox *combo : {impl->ui.difficultyCombo, impl->ui.statusCombo,
                             impl->ui.timeComplexityCombo, impl->ui.spaceComplexityCombo})
        fixComboPopup(combo);

    if (impl->ui.dateEdit)
        impl->ui.dateEdit->setDate(QDate::currentDate());

    auto *buttons = impl->findChild<QDialogButtonBox *>("dialogButtons");
    if (buttons)
    {
        auto *okButton = buttons->button(QDialogButtonBox::Ok);
        okButton->setText("Add");
        okButton->setDefault(true);
        okButton->setAutoDefault(true);
        okButton->setCursor(Qt::PointingHandCursor);
        if (auto *cancel = buttons->button(QDialogButtonBox::Cancel))
            cancel->setCursor(Qt::PointingHandCursor);

        QObject::connect(buttons, &QDialogButtonBox::accepted, impl, [impl]() {
            try
            {
                (void)impl->problem();
                impl->accept();
            }
            catch (const std::exception &error)
            {
                QMessageBox::warning(impl, "Invalid problem", error.what());
            }
        });
        QObject::connect(buttons, &QDialogButtonBox::rejected, impl, &QDialog::reject);
    }

    impl->validateInputFields();
    QObject::connect(impl->ui.nameEdit, &QLineEdit::textChanged, impl, &AddProblemDialogImpl::validateInputFields);
    QObject::connect(impl->ui.platformEdit, &QLineEdit::textChanged, impl, &AddProblemDialogImpl::validateInputFields);
    QObject::connect(impl->ui.tagsEdit, &QLineEdit::textChanged, impl, &AddProblemDialogImpl::validateInputFields);
    QObject::connect(impl->ui.nameEdit, &QLineEdit::textEdited, impl, [impl]() { impl->nameFromUrl = false; });
    QObject::connect(impl->ui.platformEdit, &QLineEdit::textEdited, impl, [impl]() { impl->platformFromUrl = false; });
    QObject::connect(impl->ui.urlEdit, &QLineEdit::textChanged, impl, [impl]() { impl->applyUrlAutofill(); });
    QObject::connect(impl->ui.urlEdit, &QLineEdit::editingFinished, impl, [impl]() { impl->applyUrlAutofill(); });
    QObject::connect(impl->ui.autoFillButton, &QPushButton::clicked, impl, [impl]() { impl->applyUrlAutofill(); });
    impl->ui.autoFillButton->setCursor(Qt::PointingHandCursor);
    impl->ui.urlEdit->setFocus();

    QObject::connect(impl->ui.browseSolutionButton, &QPushButton::clicked, impl, [impl]() {
        const QString path = QFileDialog::getOpenFileName(
            impl, "Solution file", QString(),
            "C++ sources (*.cpp *.cc *.cxx *.h *.hpp);;All files (*.*)");
        if (!path.isEmpty())
            impl->ui.solutionEdit->setText(path);
    });
    impl->ui.browseSolutionButton->setCursor(Qt::PointingHandCursor);

    if (impl->presetMinutes >= 0)
        impl->ui.timeSpin->setValue(impl->presetMinutes);
}
}

AddProblemDialog::unique_ptr AddProblemDialog::create(QWidget *parent, int presetTimeMinutes)
{
    auto *impl = new AddProblemDialogImpl(parent);
    impl->presetMinutes = presetTimeMinutes;
    auto dlg = unique_ptr{impl};
    configureDialog(impl);
    return dlg;
}

AddProblemDialog::unique_ptr AddProblemDialog::edit(const Problem &existing, QWidget *parent)
{
    auto *impl = new AddProblemDialogImpl(parent);
    auto dlg = unique_ptr{impl};
    configureDialog(impl);
    impl->loadExisting(existing);
    return dlg;
}

Problem AddProblemDialog::problem() const
{
    const auto *impl = static_cast<const AddProblemDialogImpl *>(this);
    return std::make_from_tuple<Problem>(impl->gatherData());
}
