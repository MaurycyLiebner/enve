#ifndef QREALPOINTVALUEDIALOG_H
#define QREALPOINTVALUEDIALOG_H
#include "qrealanimator.h"
#include <QDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QPushButton>

class QrealPointValueDialog : public QDialog
{
    Q_OBJECT
public:
    QrealPointValueDialog(QrealPoint *point, QWidget *parent);

private slots:
    void setValue(double value);
    void setFrame(double frame);
    void cancel();
    void ok();
signals:
    void repaintSignal();
private:
    QPushButton *mOkButton;
    QPushButton *mCancelButton;

    QVBoxLayout *mMainLayout;
    QHBoxLayout *mSpinLayout;
    QHBoxLayout *mButtonsLayout;

    void resetValue();
    QrealPoint *mPoint;

    qreal mSavedValue;
    qreal mSavedFrame;

    QDoubleSpinBox *mValueSpinBox;
    QDoubleSpinBox *mFrameSpinBox;
};

#endif // QREALPOINTVALUEDIALOG_H
