#include "AddProblemDialog.h"
#include "Problem.h"
#include "Enums.h"
#include "ui_AddProblemDialog.h"

#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <tuple>

// Implementarea privată: Găzduiește widget-urile și logica internă
class AddProblemDialogImpl final : public AddProblemDialog
{
    Q_OBJECT
    friend class AddProblemDialog;

    // Generatorul nativ Qt - acesta deține toate widget-urile (nameEdit, dateEdit, etc.)
    Ui::AddProblemDialog ui;

public:
    explicit AddProblemDialogImpl(QWidget *parent) : AddProblemDialog(parent)
    {
        // 1. Asamblarea UI-ului nativ
        ui.setupUi(this);

        // 2. TYPE SAFETY (Validare strictă pe widget-uri)
        QRegularExpression rxNotEmpty("^(?!\\s*$).+");
        ui.nameEdit->setValidator(new QRegularExpressionValidator(rxNotEmpty, this));
        ui.platformEdit->setValidator(new QRegularExpressionValidator(rxNotEmpty, this));

        QRegularExpression rxTags("^[^|]+$");
        ui.tagsEdit->setValidator(new QRegularExpressionValidator(rxTags, this));

        ui.dateEdit->setDate(QDate::currentDate());

        // Conectarea semnalelor prin obiectul UI generator
        connect(ui.dialogButtons, &QDialogButtonBox::accepted, this, &AddProblemDialogImpl::onAccepted);
        connect(ui.dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

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

private slots:
    void onAccepted()
    {
        try
        {
            // Re-validare finală la construirea obiectului
            (void)std::make_from_tuple<Problem>(gatherData());
            accept();
        }
        catch (const std::exception &e)
        {
            QMessageBox::warning(this, "Problemă invalidă", e.what());
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
    return unique_ptr{new AddProblemDialogImpl(parent)};
}

Problem AddProblemDialog::problem() const
{
    const auto *impl = static_cast<const AddProblemDialogImpl *>(this);
    return std::make_from_tuple<Problem>(impl->gatherData());
}

#include "AddProblemDialog.moc"