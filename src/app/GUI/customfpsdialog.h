#ifndef CUSTOMFPSDIALOG_H
#define CUSTOMFPSDIALOG_H
#include <QDialog>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>

class CustomFpsDialog : public QDialog
{
public:
    CustomFpsDialog();

    void setFps(const qreal fps) {
        mSpinBox->setValue(fps);
    }
    void setEnabled(const bool bT) {
        mCheckBox->setChecked(bT);
    }

    qreal getFps() {
        return mSpinBox->value();
    }

    bool getFpsEnabled() {
        return mCheckBox->isChecked();
    }
protected:
    QVBoxLayout *mMainLayout = nullptr;

    QHBoxLayout *mFpsLayout = nullptr;

    QLabel *mFpsLabel = nullptr;

    QCheckBox *mCheckBox = nullptr;
    QDoubleSpinBox *mSpinBox = nullptr;

    QHBoxLayout *mButtonsLayout = nullptr;
    QPushButton *mAcceptButton = nullptr;
    QPushButton *mCancelButton = nullptr;
};

#endif // CUSTOMFPSDIALOG_H
