#ifndef DURATIONRECTSETTINGSDIALOG_H
#define DURATIONRECTSETTINGSDIALOG_H
#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include "durationrectangle.h"
class QHBoxLayout;
class QVBoxLayout;
class TwoColumnLayout;

class DurationRectSettingsDialog : public QDialog {
    Q_OBJECT
public:
    DurationRectSettingsDialog(const DurationRectangle::Type &type,
                               const int minFrame,
                               const int maxFrame,
                               const int firstAnimationFrame,
                               QWidget *parent = nullptr);
    DurationRectSettingsDialog(const DurationRectangle::Type &type,
                               const int minFrame,
                               const int maxFrame,
                               QWidget *parent = nullptr);

    int getMinFrame() {
        return mMinFrameSpinBox->value();
    }

    int getMaxFrame() {
        return mMaxFrameSpinBox->value();
    }

    int getFirstAnimationFrame() {
        if(!mFirstAnimationFrameSpinBox) return 0;
        return mFirstAnimationFrameSpinBox->value();
    }
protected:
    DurationRectangle::Type mType;
    QVBoxLayout *mMainLayout;
    TwoColumnLayout *mTwoColumnLayout;
    QLabel *mMinFrameLabel;
    QSpinBox *mMinFrameSpinBox;
    QLabel *mMaxFrameLabel;
    QSpinBox *mMaxFrameSpinBox;
    QLabel *mFirstAnimationFrameLabel;
    QSpinBox *mFirstAnimationFrameSpinBox = nullptr;

    QHBoxLayout *mButtonsLayout;
    QPushButton *mOkButton;
    QPushButton *mCancelButton;
protected slots:
    void setMaxFrameSpinBoxMinFrame(const int frame);
    void setMinFrameSpinBoxMaxFrame(const int frame);
};

#endif // DURATIONRECTSETTINGSDIALOG_H
