#ifndef ALGOTRACK_ADD_PROBLEM_DIALOG_H
#define ALGOTRACK_ADD_PROBLEM_DIALOG_H

#include <QDialog>
#include <memory>

class Problem;
class AddProblemDialogImpl;

class AddProblemDialog : public QDialog
{
    Q_OBJECT
    friend class AddProblemDialogImpl;

    // Constructorul este PRIVAT. Nimeni din exterior nu poate instanția direct.
    explicit AddProblemDialog(QWidget *parent);

public:
    Problem problem() const;

    struct deleter
    {
        void operator()(AddProblemDialog *ptr);
    };

    using unique_ptr = std::unique_ptr<AddProblemDialog, deleter>;

    // Factory Method
    static unique_ptr create(QWidget *parent = nullptr);
};

#endif // ALGOTRACK_ADD_PROBLEM_DIALOG_H