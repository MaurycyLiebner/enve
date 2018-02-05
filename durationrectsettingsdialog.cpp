#include "durationrectsettingsdialog.h"
#include "twocolumnlayout.h"


DurationRectSettingsDialog::DurationRectSettingsDialog(
        const DurationRectangleMovable::Type &type,
        const int &minFrame,
        const int &maxFrame,
        const int &firstAnimationFrame,
        QWidget *parent) :
    QDialog(parent) {
    mType = type;
    mMainLayout = new QVBoxLayout(this);
    mTwoColumnLayout = new TwoColumnLayout();

    mMinFrameLabel = new QLabel("Min frame:");
    mMinFrameSpinBox = new QSpinBox(this);
    mMinFrameSpinBox->setRange(-99999, maxFrame);
    mMinFrameSpinBox->setValue(minFrame);
    mMaxFrameLabel = new QLabel("Max frame:");
    mMaxFrameSpinBox = new QSpinBox(this);
    mMaxFrameSpinBox->setRange(minFrame, 99999);
    mMaxFrameSpinBox->setValue(maxFrame);
    connect(mMinFrameSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setMinFrameSpinBoxMaxFrame(int)));
    connect(mMaxFrameSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(setMaxFrameSpinBoxMinFrame(int)));
    mTwoColumnLayout->addPair(mMinFrameLabel,
                              mMinFrameSpinBox);
    mTwoColumnLayout->addPair(mMaxFrameLabel,
                              mMaxFrameSpinBox);

    if(type == DurationRectangle::FIXED_LEN_ANIMATION_LEN) {
        mFirstAnimationFrameLabel = new QLabel("First animation frame:");
        mFirstAnimationFrameSpinBox = new QSpinBox(this);
        mFirstAnimationFrameSpinBox->setRange(-99999, 99999);
        mFirstAnimationFrameSpinBox->setValue(firstAnimationFrame);
        mTwoColumnLayout->addPair(mFirstAnimationFrameLabel,
                                  mFirstAnimationFrameSpinBox);
    }

    mButtonsLayout = new QHBoxLayout();
    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mButtonsLayout->addWidget(mOkButton);
    mButtonsLayout->addWidget(mCancelButton);
    mButtonsLayout->setAlignment(Qt::AlignRight);
    connect(mOkButton, SIGNAL(pressed()),
            this, SLOT(accept()));
    connect(mCancelButton, SIGNAL(pressed()),
            this, SLOT(reject()));

    mMainLayout->addLayout(mTwoColumnLayout);
    mMainLayout->addLayout(mButtonsLayout);
    setLayout(mMainLayout);
}

void DurationRectSettingsDialog::setMinFrameSpinBoxMaxFrame(const int &frame) {
    mMinFrameSpinBox->setMaximum(frame);
}

void DurationRectSettingsDialog::setMaxFrameSpinBoxMinFrame(const int &frame) {
    mMaxFrameSpinBox->setMinimum(frame);
}

DurationRectSettingsDialog::DurationRectSettingsDialog(
        const DurationRectangleMovable::Type &type,
        const int &minFrame,
        const int &maxFrame,
        QWidget *parent) :
    DurationRectSettingsDialog(type, minFrame,
                               maxFrame, 0, parent) {}
