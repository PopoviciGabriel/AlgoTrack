#include "AddProblemDialog.h"
#include "Problem.h"
#include "Enums.h"
#include "ui_AddProblemDialog.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QStringList>
#include <QPushButton>
#include <tuple>

// Implementarea privată: Găzduiește cache-ul widgeturilor și logica de extragere
class AddProblemDialogImpl final : public AddProblemDialog
{
    Q_OBJECT
    friend class AddProblemDialog;

public:
    // Păstrăm pointerii aici pentru a nu mai apela findChild de 9 ori
    QLineEdit *nameEdit = nullptr;
    QLineEdit *platformEdit = nullptr;
    QComboBox *difficultyCombo = nullptr;
    QLineEdit *tagsEdit = nullptr;
    QComboBox *statusCombo = nullptr;
    QSpinBox *timeSpin = nullptr;
    QDateEdit *dateEdit = nullptr;
    QDoubleSpinBox *ratingSpin = nullptr;
    QPlainTextEdit *notesEdit = nullptr;

    explicit AddProblemDialogImpl(QWidget *parent) : AddProblemDialog(parent) {}

    // Împachetăm datele într-un tuple curat
    auto gatherData() const
    {
        QStringList rawTags = tagsEdit->text().split(',', Qt::SkipEmptyParts);
        std::vector<std::string> tags;
        tags.reserve(rawTags.size());
        for (const QString &tag : rawTags)
        {
            tags.push_back(tag.trimmed().toStdString());
        }

        Difficulty diff = (difficultyCombo->currentIndex() == 0) ? Difficulty::Easy : (difficultyCombo->currentIndex() == 1) ? Difficulty::Medium
                                                                                                                             : Difficulty::Hard;

        Status stat = (statusCombo->currentIndex() == 0) ? Status::Solved : (statusCombo->currentIndex() == 1) ? Status::Failed
                                                                                                               : Status::InProgress;

        return std::make_tuple(
            nameEdit->text().toStdString(),
            platformEdit->text().toStdString(),
            diff,
            tags,
            stat,
            timeSpin->value(),
            dateEdit->date().toString("dd-MM-yyyy").toStdString(),
            ratingSpin->value(),
            notesEdit->toPlainText().toStdString());
    }
};

AddProblemDialog::AddProblemDialog(QWidget *parent) : QDialog(parent) {}

void AddProblemDialog::deleter::operator()(AddProblemDialog *ptr)
{
    delete static_cast<AddProblemDialogImpl *>(ptr);
}

// FACTORY METHOD: Alocă, asamblează, configurează și returnează
// FACTORY METHOD: Alocă, asamblează, configurează și returnează
AddProblemDialog::unique_ptr AddProblemDialog::create(QWidget *parent)
{
    // 1. Instanțiem direct clasa ascunsă pentru a avea acces la membrii ei
    auto *impl = new AddProblemDialogImpl(parent);

    // 2. O ambalăm imediat în smart pointer-ul cerut de interfață pentru a garanta siguranța memoriei
    auto dlg = unique_ptr{impl};

    Ui::AddProblemDialog ui;
    ui.setupUi(impl);

    // UI Styles
    impl->setStyleSheet(R"(
        QDialog { background: #f7f8fb; color: #172033; }
        QLabel#dialogTitle { font-size: 22px; font-weight: 700; color: #111827; }
        QLabel#dialogSubtitle { color: #667085; }
        QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox, QPlainTextEdit {
            min-height: 34px; border: 1px solid #d7dce5; border-radius: 8px; padding: 6px 10px; background: white; selection-background-color: #2563eb;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QPlainTextEdit:focus { border: 1px solid #2563eb; }
        QPushButton { min-width: 86px; min-height: 34px; border-radius: 8px; padding: 7px 14px; border: 1px solid #d7dce5; background: white; color: #111827; }
        QPushButton:hover { background: #eef4ff; border-color: #b7c9f8; }
    )");

    // Caching-ul pointerilor folosind instanța 'impl'
    impl->nameEdit = impl->findChild<QLineEdit *>("nameEdit");
    impl->platformEdit = impl->findChild<QLineEdit *>("platformEdit");
    impl->difficultyCombo = impl->findChild<QComboBox *>("difficultyCombo");
    impl->tagsEdit = impl->findChild<QLineEdit *>("tagsEdit");
    impl->statusCombo = impl->findChild<QComboBox *>("statusCombo");
    impl->timeSpin = impl->findChild<QSpinBox *>("timeSpin");
    impl->dateEdit = impl->findChild<QDateEdit *>("dateEdit");
    impl->ratingSpin = impl->findChild<QDoubleSpinBox *>("ratingSpin");
    impl->notesEdit = impl->findChild<QPlainTextEdit *>("notesEdit");

    if (!impl->nameEdit || !impl->platformEdit || !impl->difficultyCombo || !impl->tagsEdit ||
        !impl->statusCombo || !impl->timeSpin || !impl->dateEdit || !impl->ratingSpin || !impl->notesEdit)
    {
        throw std::runtime_error("UI decomposition error: some fields are missing.");
    }

    auto *buttons = impl->findChild<QDialogButtonBox *>("dialogButtons");
    if (buttons)
    {
        buttons->button(QDialogButtonBox::Ok)->setText("Add");

        // Conectăm butoanele la validarea internă, folosind tot 'impl'
        QObject::connect(buttons, &QDialogButtonBox::accepted, impl, [impl]()
                         {
            try {
                (void)impl->problem(); // Aruncă excepție dacă datele nu sunt valide
                impl->accept();
            } catch (const std::exception& e) {
                QMessageBox::warning(impl, "Invalid problem", e.what());
            } });
        QObject::connect(buttons, &QDialogButtonBox::rejected, impl, &QDialog::reject);
    }

    if (impl->dateEdit)
    {
        impl->dateEdit->setDate(QDate::currentDate());
    }

    // Returnăm instanța mascată sub interfața de bază
    return dlg;
}

Problem AddProblemDialog::problem() const
{
    const auto *impl = static_cast<const AddProblemDialogImpl *>(this);

    // Tuple Application: Aplică direct tuple-ul pe constructorul clasei Problem
    return std::make_from_tuple<Problem>(impl->gatherData());
}

#include "AddProblemDialog.moc"