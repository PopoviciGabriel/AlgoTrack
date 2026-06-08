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

class AddProblemDialogImpl final : public AddProblemDialog
{
    Q_OBJECT
    friend class AddProblemDialog;

public:
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

AddProblemDialog::unique_ptr AddProblemDialog::create(QWidget *parent)
{
    auto *impl = new AddProblemDialogImpl(parent);
    auto dlg = unique_ptr{impl};

    Ui::AddProblemDialog ui;
    ui.setupUi(impl);

    // Am eliminat complet stilul hardcodat de aici. Dialogul va asculta perfect de tema globală.

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

    if (impl->dateEdit)
    {
        impl->dateEdit->setDate(QDate::currentDate());
    }

    return dlg;
}

Problem AddProblemDialog::problem() const
{
    const auto *impl = static_cast<const AddProblemDialogImpl *>(this);
    return std::make_from_tuple<Problem>(impl->gatherData());
}

#include "AddProblemDialog.moc"