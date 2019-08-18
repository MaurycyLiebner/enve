#ifndef DURATIONRECTSETTINGSDIALOG_H
#define DURATIONRECTSETTINGSDIALOG_H
#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include "Timeline/durationrectangle.h"
class QHBoxLayout;
class QVBoxLayout;
class TwoColumnLayout;

class DurationRectSettingsDialog : public QDialog {
public:
    DurationRectSettingsDialog(const int shift,
                               const int minFrame,
                               const int maxFrame,
                               QWidget *parent = nullptr);

    int getMinFrame() const {
        return mMinFrameSpinBox->value();
    }

    int getMaxFrame() const {
        return mMaxFrameSpinBox->value();
    }

    int getShift() const {
        return mShiftSpinBox->value();
    }
protected:
    QVBoxLayout *mMainLayout;
    TwoColumnLayout *mTwoColumnLayout;

    QLabel *mShiftLabel;
    QSpinBox *mShiftSpinBox;

    QLabel *mMinFrameLabel;
    QSpinBox *mMinFrameSpinBox;

    QLabel *mMaxFrameLabel;
    QSpinBox *mMaxFrameSpinBox;

    QLabel *mFirstAnimationFrameLabel;
    QSpinBox *mFirstAnimationFrameSpinBox = nullptr;

    QHBoxLayout *mButtonsLayout;
    QPushButton *mOkButton;
    QPushButton *mCancelButton;
};

#endif // DURATIONRECTSETTINGSDIALOG_H
