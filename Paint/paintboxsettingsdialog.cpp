#include "paintboxsettingsdialog.h"
#include <QPushButton>

int PaintBoxSettingsDialog::mLastFirstFrame = 0;
int PaintBoxSettingsDialog::mLastFrameStep = 2;
int PaintBoxSettingsDialog::mLastFrameCount = 100;
int PaintBoxSettingsDialog::mLastOverlapFrames = 1;

PaintBoxSettingsDialog::PaintBoxSettingsDialog(QWidget *parent) :
    QDialog(parent) {
    mMainLayout = new QVBoxLayout();
    setLayout(mMainLayout);

    mFirstFrameLayout = new QHBoxLayout();
    mFrameStepLayout = new QHBoxLayout();
    mFrameCountLayout = new QHBoxLayout();
    mOverlapFramesLayout = new QHBoxLayout();

    mMainLayout->addLayout(mFirstFrameLayout);
    mMainLayout->addLayout(mFrameStepLayout);
    mMainLayout->addLayout(mFrameCountLayout);
    mMainLayout->addLayout(mOverlapFramesLayout);

    mFirstFrameLabel = new QLabel("First Frame: ", this);
    mFrameStepLabel = new QLabel("Frame Step: ", this);
    mFrameCountLabel = new QLabel("Frame Count: ", this);
    mOverlapFramesLabel = new QLabel("Overlap Frame: ", this);
    mFirstFrameSpin = new QSpinBox(this);
    mFirstFrameSpin->setRange(0, 99999);
    mFrameStepSpin = new QSpinBox(this);
    mFrameStepSpin->setRange(1, 99);
    mFrameCountSpin = new QSpinBox(this);
    mFrameCountSpin->setRange(0, 99999);
    mOverlapFramesSpin = new QSpinBox(this);
    mOverlapFramesSpin->setRange(0, 99);

    mFirstFrameLayout->addWidget(mFirstFrameLabel);
    mFrameStepLayout->addWidget(mFrameStepLabel);
    mFrameCountLayout->addWidget(mFrameCountLabel);
    mOverlapFramesLayout->addWidget(mOverlapFramesLabel);

    mFirstFrameLayout->addWidget(mFirstFrameSpin);
    mFrameStepLayout->addWidget(mFrameStepSpin);
    mFrameCountLayout->addWidget(mFrameCountSpin);
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
    if(result() == QDialog::Accepted) {
        mLastFirstFrame = getFirstFrame();
        mLastFrameStep = getFrameStep();
        mLastFrameCount = getFrameCount();
        mLastOverlapFrames = getOverlapFrames();
    }
}

int PaintBoxSettingsDialog::getFirstFrame() {
    return mFirstFrameSpin->value();
}

int PaintBoxSettingsDialog::getFrameStep() {
    return mFrameStepSpin->value();
}

int PaintBoxSettingsDialog::getFrameCount() {
    return mFrameCountSpin->value();
}

int PaintBoxSettingsDialog::getOverlapFrames() {
    return mOverlapFramesSpin->value();
}
