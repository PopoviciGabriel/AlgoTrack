#ifndef ALGOTRACK_ADD_PROBLEM_DIALOG_H
#define ALGOTRACK_ADD_PROBLEM_DIALOG_H

#include <QDialog>
#include <memory>

class Problem;
class AddProblemDialogImpl; // Forward declaration pentru implementarea ascunsă

class AddProblemDialog : public QDialog
{
    Q_OBJECT
    friend class AddProblemDialogImpl;

    // Constructorul este PRIVAT. Nimeni din exterior nu poate face `AddProblemDialog dialog;`
    explicit AddProblemDialog(QWidget *parent);

public:
    Problem problem() const;

    // Deleter personalizat pentru un unique_ptr capabil să distrugă tipul ascuns
    // fără să fim obligați să adăugăm un dtor virtual costisitor
    struct deleter
    {
        void operator()(AddProblemDialog *ptr);
    };

    using unique_ptr = std::unique_ptr<AddProblemDialog, deleter>;

    // Factory Method: Singura metodă prin care MainWindow va instanția dialogul
    static unique_ptr create(QWidget *parent = nullptr);
};

#endif // ALGOTRACK_ADD_PROBLEM_DIALOG_H
