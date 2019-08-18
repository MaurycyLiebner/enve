#include "durationrectsettingsdialog.h"
#include "twocolumnlayout.h"


DurationRectSettingsDialog::DurationRectSettingsDialog(
        const int shift,
        const int minFrame,
        const int maxFrame,
        QWidget *parent) :
    QDialog(parent) {
    mMainLayout = new QVBoxLayout(this);
    mTwoColumnLayout = new TwoColumnLayout();

    mShiftLabel = new QLabel("Frame shift:");
    mShiftSpinBox = new QSpinBox(this);
    mShiftSpinBox->setRange(-99999, 99999);
    mShiftSpinBox->setValue(shift);
    mTwoColumnLayout->addPair(mShiftLabel, mShiftSpinBox);

    mMinFrameLabel = new QLabel("Min frame:");
    mMinFrameSpinBox = new QSpinBox(this);
    mMinFrameSpinBox->setRange(-99999, maxFrame);
    mMinFrameSpinBox->setValue(minFrame);
    mTwoColumnLayout->addPair(mMinFrameLabel, mMinFrameSpinBox);

    mMaxFrameLabel = new QLabel("Max frame:");
    mMaxFrameSpinBox = new QSpinBox(this);
    mMaxFrameSpinBox->setRange(minFrame, 99999);
    mMaxFrameSpinBox->setValue(maxFrame);
    mTwoColumnLayout->addPair(mMaxFrameLabel, mMaxFrameSpinBox);

    connect(mMinFrameSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            mMaxFrameSpinBox, &QSpinBox::setMinimum);
    connect(mMaxFrameSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            mMinFrameSpinBox, &QSpinBox::setMaximum);

    mButtonsLayout = new QHBoxLayout();
    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mButtonsLayout->addWidget(mOkButton);
    mButtonsLayout->addWidget(mCancelButton);
    mButtonsLayout->setAlignment(Qt::AlignRight);
    connect(mOkButton, &QPushButton::pressed, this, &QDialog::accept);
    connect(mCancelButton, &QPushButton::pressed, this, &QDialog::reject);

    mMainLayout->addLayout(mTwoColumnLayout);
    mMainLayout->addLayout(mButtonsLayout);
    setLayout(mMainLayout);
}
