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
    int getCanvasFrameCount();
private:
    QVBoxLayout *mMainLayout;

    QHBoxLayout *mNameLayout;
    QLabel *mNameEditLabel;
    QLineEdit *mNameEdit;

    QHBoxLayout *mSizeLayout;
    QLabel *mWidthLabel;
    QSpinBox *mWidthSpinBox;
    QLabel *mHeightLabel;
    QSpinBox *mHeightSpinBox;

    QHBoxLayout *mFrameCountLayout;
    QLabel *mFrameCountLabel;
    QSpinBox *mFrameCountSpinBox;

    QPushButton *mOkButton;
    QPushButton *mCancelButton;
    QHBoxLayout *mButtonsLayout;
};

#endif // NEWCANVASDIALOG_H
