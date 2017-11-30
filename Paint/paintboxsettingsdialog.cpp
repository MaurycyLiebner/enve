#include "paintboxsettingsdialog.h"
#include <QPushButton>

PaintBoxSettingsDialog::PaintBoxSettingsDialog(QWidget *parent) :
    QDialog(parent) {
    mMainLayout = new QVBoxLayout();
    setLayout(mMainLayout);

    mFrameStepLayout = new QHBoxLayout();
    mOverlapFramesLayout = new QHBoxLayout();

    mMainLayout->addLayout(mFrameStepLayout);
    mMainLayout->addLayout(mOverlapFramesLayout);

    mFrameStepLabel = new QLabel("Frame step: ", this);
    mOverlapFramesLabel = new QLabel("Overlap frames: ", this);
    mFrameStepSpin = new QSpinBox(this);
    mFrameStepSpin->setRange(1, 99);
    mOverlapFramesSpin = new QSpinBox(this);
    mOverlapFramesSpin->setRange(0, 99);

    mFrameStepLayout->addWidget(mFrameStepLabel);
    mOverlapFramesLayout->addWidget(mOverlapFramesLabel);

    mFrameStepLayout->addWidget(mFrameStepSpin);
    mOverlapFramesLayout->addWidget(mOverlapFramesSpin);

    mButtonsLayout = new QHBoxLayout();
    mMainLayout->addLayout(mButtonsLayout);
    mAcceptButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mButtonsLayout->addWidget(mAcceptButton);
    mButtonsLayout->addWidget(mCancelButton);
    connect(mAcceptButton, SIGNAL(released()),
            this, SLOT(accept()));
    connect(mCancelButton, SIGNAL(released()),
            this, SLOT(reject()));
}

PaintBoxSettingsDialog::~PaintBoxSettingsDialog() {
}

int PaintBoxSettingsDialog::getFrameStep() {
    return mFrameStepSpin->value();
}

int PaintBoxSettingsDialog::getOverlapFrames() {
    return mOverlapFramesSpin->value();
}
