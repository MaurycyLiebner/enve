#ifndef EXPRESSIONDIALOG_H
#define EXPRESSIONDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class QrealAnimator;
class ExpressionEditor;

class ExpressionDialog : public QDialog {
public:
    ExpressionDialog(QrealAnimator* const target,
                     QWidget * const parent = nullptr);

private:
    void apply();

    QrealAnimator* const mTarget;

    ExpressionEditor* mLine;
    QLabel* mErrorLabel;
};

#endif // EXPRESSIONDIALOG_H
