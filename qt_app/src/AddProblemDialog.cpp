#include "AddProblemDialog.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QSizeGrip>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QVBoxLayout>

AddProblemDialog::AddProblemDialog(QWidget* parent)
    : QDialog(parent),
      nameEdit(new QLineEdit(this)),
      platformEdit(new QLineEdit(this)),
      difficultyCombo(new QComboBox(this)),
      tagsEdit(new QLineEdit(this)),
      statusCombo(new QComboBox(this)),
      timeSpin(new QSpinBox(this)),
      dateEdit(new QDateEdit(QDate::currentDate(), this)),
      ratingSpin(new QDoubleSpinBox(this)),
      notesEdit(new QPlainTextEdit(this)),
      buttons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this)) {
    setWindowTitle("Add problem");
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
    setSizeGripEnabled(true);
    setMinimumSize(500, 560);
    resize(560, 640);

    difficultyCombo->addItems({"Easy", "Medium", "Hard"});
    statusCombo->addItems({"Solved", "Failed", "In Progress"});
    tagsEdit->setPlaceholderText("hashmap, arrays, graphs");

    timeSpin->setRange(0, 100000);
    timeSpin->setSuffix(" min");

    dateEdit->setDisplayFormat("dd-MM-yyyy");
    dateEdit->setCalendarPopup(true);

    ratingSpin->setRange(1.0, 10.0);
    ratingSpin->setSingleStep(0.5);
    ratingSpin->setDecimals(1);
    ratingSpin->setValue(5.0);

    notesEdit->setMinimumHeight(110);

    auto* title = new QLabel("New problem", this);
    title->setObjectName("dialogTitle");

    auto* subtitle = new QLabel("Track the problem details cleanly now, analyze them later.", this);
    subtitle->setObjectName("dialogSubtitle");

    auto* form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(12);
    form->addRow("Name", nameEdit);
    form->addRow("Platform", platformEdit);
    form->addRow("Difficulty", difficultyCombo);
    form->addRow("Tags", tagsEdit);
    form->addRow("Status", statusCombo);
    form->addRow("Time spent", timeSpin);
    form->addRow("Date", dateEdit);
    form->addRow("Rating", ratingSpin);
    form->addRow("Notes", notesEdit);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 22, 24, 20);
    layout->setSpacing(12);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(8);
    layout->addLayout(form);
    layout->addSpacing(8);
    layout->addWidget(buttons);
    layout->setStretchFactor(form, 1);

    setStyleSheet(R"(
        QDialog {
            background: #f7f8fb;
            color: #172033;
        }
        QLabel#dialogTitle {
            font-size: 22px;
            font-weight: 700;
            color: #111827;
        }
        QLabel#dialogSubtitle {
            color: #667085;
        }
        QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox, QPlainTextEdit {
            min-height: 34px;
            border: 1px solid #d7dce5;
            border-radius: 8px;
            padding: 6px 10px;
            background: white;
            selection-background-color: #2563eb;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus,
        QSpinBox:focus, QDoubleSpinBox:focus, QPlainTextEdit:focus {
            border: 1px solid #2563eb;
        }
        QPushButton {
            min-width: 86px;
            min-height: 34px;
            border-radius: 8px;
            padding: 7px 14px;
            border: 1px solid #d7dce5;
            background: white;
            color: #111827;
        }
        QPushButton:hover {
            background: #eef4ff;
            border-color: #b7c9f8;
        }
    )");

    buttons->button(QDialogButtonBox::Ok)->setText("Add");

    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        try {
            (void)problem();
            accept();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Invalid problem", e.what());
        }
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

Problem AddProblemDialog::problem() const {
    QStringList rawTags = tagsEdit->text().split(',', Qt::SkipEmptyParts);
    std::vector<std::string> tags;
    tags.reserve(rawTags.size());

    for (const QString& tag : rawTags) {
        tags.push_back(tag.trimmed().toStdString());
    }

    return Problem(
        nameEdit->text().toStdString(),
        platformEdit->text().toStdString(),
        selectedDifficulty(),
        tags,
        selectedStatus(),
        timeSpin->value(),
        dateEdit->date().toString("dd-MM-yyyy").toStdString(),
        ratingSpin->value(),
        notesEdit->toPlainText().toStdString()
    );
}

Difficulty AddProblemDialog::selectedDifficulty() const {
    switch (difficultyCombo->currentIndex()) {
    case 0: return Difficulty::Easy;
    case 1: return Difficulty::Medium;
    default: return Difficulty::Hard;
    }
}

Status AddProblemDialog::selectedStatus() const {
    switch (statusCombo->currentIndex()) {
    case 0: return Status::Solved;
    case 1: return Status::Failed;
    default: return Status::InProgress;
    }
}


