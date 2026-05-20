#include "AddProblemDialog.h"
#include "Problem.h"
#include "Enums.h"
#include "ui_AddProblemDialog.h" // Fișier generat automat de AUTOUIC pe baza .ui

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

// Implementarea privată ascunsă total de fișierele headers
class AddProblemDialogImpl final : public AddProblemDialog
{
    Q_OBJECT
    friend class AddProblemDialog;

    explicit AddProblemDialogImpl(QWidget *parent) : AddProblemDialog(parent)
    {
        // Constructorul e curat. Alocă memorie și gata. Fără "side effects".
    }
};

AddProblemDialog::AddProblemDialog(QWidget *parent) : QDialog(parent) {}

void AddProblemDialog::deleter::operator()(AddProblemDialog *ptr)
{
    delete static_cast<AddProblemDialogImpl *>(ptr);
}

// FACTORY METHOD: Șeful de șantier. Alocă, asamblează, configurează și returnează.
AddProblemDialog::unique_ptr AddProblemDialog::create(QWidget *parent)
{
    auto dlg = unique_ptr{new AddProblemDialogImpl{parent}};

    // Încărcăm XML-ul convertit în cod de Qt
    Ui::AddProblemDialog ui;
    ui.setupUi(dlg.get());

    // UI Styles
    dlg->setStyleSheet(R"(
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

    auto *buttons = dlg->findChild<QDialogButtonBox *>("dialogButtons");
    if (buttons)
    {
        buttons->button(QDialogButtonBox::Ok)->setText("Add");

        // Conectăm butoanele la validarea internă
        QObject::connect(buttons, &QDialogButtonBox::accepted, dlg.get(), [dlgPtr = dlg.get()]()
                         {
            try {
                (void)dlgPtr->problem(); // Aruncă excepție dacă datele nu sunt valide
                dlgPtr->accept();
            } catch (const std::exception& e) {
                QMessageBox::warning(dlgPtr, "Invalid problem", e.what());
            } });
        QObject::connect(buttons, &QDialogButtonBox::rejected, dlg.get(), &QDialog::reject);
    }

    auto *dateEdit = dlg->findChild<QDateEdit *>("dateEdit");
    if (dateEdit)
    {
        dateEdit->setDate(QDate::currentDate());
    }

    return dlg;
}

Problem AddProblemDialog::problem() const
{
    // Extragem dinamic, eliminând dependențele de memorie
    auto *nameEdit = findChild<QLineEdit *>("nameEdit");
    auto *platformEdit = findChild<QLineEdit *>("platformEdit");
    auto *difficultyCombo = findChild<QComboBox *>("difficultyCombo");
    auto *tagsEdit = findChild<QLineEdit *>("tagsEdit");
    auto *statusCombo = findChild<QComboBox *>("statusCombo");
    auto *timeSpin = findChild<QSpinBox *>("timeSpin");
    auto *dateEdit = findChild<QDateEdit *>("dateEdit");
    auto *ratingSpin = findChild<QDoubleSpinBox *>("ratingSpin");
    auto *notesEdit = findChild<QPlainTextEdit *>("notesEdit");

    if (!nameEdit || !platformEdit || !difficultyCombo || !tagsEdit || !statusCombo || !timeSpin || !dateEdit || !ratingSpin || !notesEdit)
    {
        throw std::runtime_error("UI decomposition error: some fields are missing.");
    }

    QStringList rawTags = tagsEdit->text().split(',', Qt::SkipEmptyParts);
    std::vector<std::string> tags;
    tags.reserve(rawTags.size());
    for (const QString &tag : rawTags)
    {
        tags.push_back(tag.trimmed().toStdString());
    }

    auto getDifficulty = [difficultyCombo]()
    {
        switch (difficultyCombo->currentIndex())
        {
        case 0:
            return Difficulty::Easy;
        case 1:
            return Difficulty::Medium;
        default:
            return Difficulty::Hard;
        }
    };

    auto getStatus = [statusCombo]()
    {
        switch (statusCombo->currentIndex())
        {
        case 0:
            return Status::Solved;
        case 1:
            return Status::Failed;
        default:
            return Status::InProgress;
        }
    };

    // Apelează direct constructorul complet validat (RAII)
    return Problem(
        nameEdit->text().toStdString(),
        platformEdit->text().toStdString(),
        getDifficulty(),
        tags,
        getStatus(),
        timeSpin->value(),
        dateEdit->date().toString("dd-MM-yyyy").toStdString(),
        ratingSpin->value(),
        notesEdit->toPlainText().toStdString());
}

#include "AddProblemDialog.moc"
