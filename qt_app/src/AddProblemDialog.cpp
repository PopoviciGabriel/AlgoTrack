#include "AddProblemDialog.h"
#include "ui_AddProblemDialog.h"
#include "Problem.h"
#include "Enums.h"

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
#include <QVBoxLayout>
#include <QStyledItemDelegate>
#include <QFile>
#include <tuple>
#include <QAbstractItemView>

class AddProblemDialogImpl final : public AddProblemDialog
{
    friend class AddProblemDialog;

public:
    // Nativul generat de Qt înlocuiește toți pointerii manuali de dinainte
    Ui::AddProblemDialog ui;

    explicit AddProblemDialogImpl(QWidget *parent) : AddProblemDialog(parent) {}

    auto gatherData() const
    {
        QStringList rawTags = ui.tagsEdit->text().split(',', Qt::SkipEmptyParts);
        std::vector<std::string> tags;
        tags.reserve(rawTags.size());
        for (const QString &tag : rawTags)
        {
            tags.push_back(tag.trimmed().toStdString());
        }

        Difficulty diff = (ui.difficultyCombo->currentIndex() == 0) ? Difficulty::Easy : (ui.difficultyCombo->currentIndex() == 1) ? Difficulty::Medium
                                                                                                                                   : Difficulty::Hard;

        Status stat = (ui.statusCombo->currentIndex() == 0) ? Status::Solved : (ui.statusCombo->currentIndex() == 1) ? Status::Failed
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
            ui.notesEdit->toPlainText().toStdString());
    }

    // Validare reactivă în timp real a inputului din UI
    void validateInputFields()
    {
        auto *buttons = findChild<QDialogButtonBox *>("dialogButtons");
        if (buttons)
        {
            bool hasName = !ui.nameEdit->text().trimmed().isEmpty();
            bool hasPlatform = !ui.platformEdit->text().trimmed().isEmpty();
            bool hasTags = !ui.tagsEdit->text().trimmed().isEmpty();

            buttons->button(QDialogButtonBox::Ok)->setEnabled(hasName && hasPlatform && hasTags);
        }
    }
};

AddProblemDialog::AddProblemDialog(QWidget *parent) : QDialog(parent) {}

void AddProblemDialog::deleter::operator()(AddProblemDialog *ptr)
{
    delete static_cast<AddProblemDialogImpl *>(ptr);
}

AddProblemDialog::unique_ptr AddProblemDialog::create(QWidget *parent)
{
    auto *impl = new AddProblemDialogImpl(parent);
    auto dlg = unique_ptr{impl};

    // Inițializare directă pe instanța UI deținută de Impl
    impl->ui.setupUi(impl);

    impl->ui.timeSpin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    impl->ui.ratingSpin->setButtonSymbols(QAbstractSpinBox::UpDownArrows);

    // Încărcare CSS curată din sistemul de resurse (.qrc)
    QFile styleFile(":/styles/dialog.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        impl->setStyleSheet(QLatin1String(styleFile.readAll()));
    }

    // Delegat pentru stilizare fără hack-uri de transparență
    impl->ui.difficultyCombo->setItemDelegate(new QStyledItemDelegate(impl->ui.difficultyCombo));
    impl->ui.statusCombo->setItemDelegate(new QStyledItemDelegate(impl->ui.statusCombo));

    // --- FIX DEFINITIV PENTRU FUNDALUL LISTEI DROP-DOWN ---
    auto fixComboPopup = [](QComboBox *cb)
    {
        QWidget *popup = cb->view()->window();

        popup->setWindowFlags(
            Qt::Popup |
            Qt::FramelessWindowHint |
            Qt::NoDropShadowWindowHint);

        popup->setStyleSheet(
            "border-radius: 8px;");
    };

    fixComboPopup(impl->ui.difficultyCombo);
    fixComboPopup(impl->ui.statusCombo);
    // -------------------------------------------------------

    if (impl->ui.dateEdit)
    {
        impl->ui.dateEdit->setDate(QDate::currentDate());
    }

    auto *buttons = impl->findChild<QDialogButtonBox *>("dialogButtons");
    if (buttons)
    {
        buttons->button(QDialogButtonBox::Ok)->setText("Add");

        QObject::connect(buttons, &QDialogButtonBox::accepted, impl, [impl]()
                         {
            try {
                (void)impl->problem(); 
                impl->accept();
            } catch (const std::exception& e) {
                QMessageBox::warning(impl, "Invalid problem", e.what());
            } });
        QObject::connect(buttons, &QDialogButtonBox::rejected, impl, &QDialog::reject);
    }

    // Înregistrare semnale pentru verificarea în timp real a validității
    impl->validateInputFields();
    QObject::connect(impl->ui.nameEdit, &QLineEdit::textChanged, impl, &AddProblemDialogImpl::validateInputFields);
    QObject::connect(impl->ui.platformEdit, &QLineEdit::textChanged, impl, &AddProblemDialogImpl::validateInputFields);
    QObject::connect(impl->ui.tagsEdit, &QLineEdit::textChanged, impl, &AddProblemDialogImpl::validateInputFields);

    return dlg;
}

Problem AddProblemDialog::problem() const
{
    const auto *impl = static_cast<const AddProblemDialogImpl *>(this);
    return std::make_from_tuple<Problem>(impl->gatherData());
}