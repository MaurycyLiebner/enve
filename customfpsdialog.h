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

    void setFps(const qreal &fps) {
        mSpinBox->setValue(fps);
    }
    void setEnabled(const bool &bT) {
        mCheckBox->setChecked(bT);
    }

    qreal getFps() {
        return mSpinBox->value();
    }

    bool getFpsEnabled() {
        return mCheckBox->isChecked();
    }
protected:
    QVBoxLayout *mMainLayout = NULL;

    QHBoxLayout *mFpsLayout = NULL;

    QLabel *mFpsLabel = NULL;

    QCheckBox *mCheckBox = NULL;
    QDoubleSpinBox *mSpinBox = NULL;

    QHBoxLayout *mButtonsLayout = NULL;
    QPushButton *mAcceptButton = NULL;
    QPushButton *mCancelButton = NULL;
};

#endif // CUSTOMFPSDIALOG_H
