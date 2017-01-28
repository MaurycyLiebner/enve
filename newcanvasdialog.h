#ifndef NEWCANVASDIALOG_H
#define NEWCANVASDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>

class NewCanvasDialog : public QDialog
{
public:
    NewCanvasDialog(const QString &defName, QWidget *parent = NULL);

    int getCanvasWidth();
    int getCanvasHeight();
    QString getCanvasName();
private:
    QVBoxLayout *mMainLayout;

    QHBoxLayout *mNameLayout;
    QLabel *mNameEditLabel;
    QLineEdit *mNameEdit;

    QHBoxLayout *mSizeLayout;
    QSpinBox *mWidthSpinBox;
    QSpinBox *mHeightSpinBox;

    QPushButton *mOkButton;
    QPushButton *mCancelButton;
    QHBoxLayout *mButtonsLayout;
};

#endif // NEWCANVASDIALOG_H
