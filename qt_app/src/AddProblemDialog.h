#pragma once

#include "Problem.h"

#include <QDialog>

class QComboBox;
class QDateEdit;
class QDialogButtonBox;
class QDoubleSpinBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;

class AddProblemDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddProblemDialog(QWidget* parent = nullptr);

    Problem problem() const;

private:
    QLineEdit* nameEdit;
    QLineEdit* platformEdit;
    QComboBox* difficultyCombo;
    QLineEdit* tagsEdit;
    QComboBox* statusCombo;
    QSpinBox* timeSpin;
    QDateEdit* dateEdit;
    QDoubleSpinBox* ratingSpin;
    QPlainTextEdit* notesEdit;
    QDialogButtonBox* buttons;

    Difficulty selectedDifficulty() const;
    Status selectedStatus() const;
};
