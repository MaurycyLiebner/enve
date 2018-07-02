#include "newcanvasdialog.h"
#include "canvas.h"
#include "BoxesList/coloranimatorbutton.h"

CanvasSettingsDialog::CanvasSettingsDialog(Canvas *canvas,
                                           QWidget *parent) :
    CanvasSettingsDialog(canvas->getName(),
                         canvas->getCanvasWidth(),
                         canvas->getCanvasHeight(),
                         canvas->getMaxFrame(),
                         canvas->getFps(),
                         canvas->getBgColorAnimator(),
                         parent) {}

CanvasSettingsDialog::CanvasSettingsDialog(const QString &defName,
                                           QWidget *parent) :
    CanvasSettingsDialog(defName, 1920, 1080, 200, 24., nullptr, parent) {}

CanvasSettingsDialog::CanvasSettingsDialog(const QString &currName,
                                           const int &currWidth,
                                           const int &currHeight,
                                           const int &currFrameCount,
                                           const qreal &currFps,
                                           ColorAnimator *bgColorAnimator,
                                           QWidget *parent) :
    QDialog(parent) {
    setWindowTitle("Canvas Settings");
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mNameLayout = new QHBoxLayout();
    mNameEditLabel = new QLabel("Name: ", this);
    mNameEdit = new QLineEdit(currName, this);
    mNameLayout->addWidget(mNameEditLabel);
    mNameLayout->addWidget(mNameEdit);
    mMainLayout->addLayout(mNameLayout);

    mWidthLabel = new QLabel("Width:", this);
    mWidthSpinBox = new QSpinBox(this);
    mWidthSpinBox->setRange(1, 9999);
    mWidthSpinBox->setValue(currWidth);

    mHeightLabel = new QLabel("Height:", this);
    mHeightSpinBox = new QSpinBox(this);
    mHeightSpinBox->setRange(1, 9999);
    mHeightSpinBox->setValue(currHeight);

    mSizeLayout = new QHBoxLayout();
    mSizeLayout->addWidget(mWidthLabel);
    mSizeLayout->addWidget(mWidthSpinBox);
    mSizeLayout->addWidget(mHeightLabel);
    mSizeLayout->addWidget(mHeightSpinBox);
    mMainLayout->addLayout(mSizeLayout);

    mFrameCountLabel = new QLabel("Frame Count:", this);
    mFrameCountSpinBox = new QSpinBox(this);
    mFrameCountSpinBox->setRange(1, 9999);
    mFrameCountSpinBox->setValue(currFrameCount);

    mFrameCountLayout = new QHBoxLayout();
    mFrameCountLayout->addWidget(mFrameCountLabel);
    mFrameCountLayout->addWidget(mFrameCountSpinBox);
    mMainLayout->addLayout(mFrameCountLayout);

    mFPSLabel = new QLabel("Fps:", this);
    mFPSSpinBox = new QDoubleSpinBox(this);
    mFPSSpinBox->setRange(1., 300.);
    mFPSSpinBox->setValue(currFps);

    mFPSLayout = new QHBoxLayout();
    mFPSLayout->addWidget(mFPSLabel);
    mFPSLayout->addWidget(mFPSSpinBox);
    mMainLayout->addLayout(mFPSLayout);

    mBgColorLabel = new QLabel("Backgroud:", this);
    mBgColorButton = new ColorAnimatorButton(bgColorAnimator, this);

    mBgColorLayout = new QHBoxLayout();
    mBgColorLayout->addWidget(mBgColorLabel);
    mBgColorLayout->addWidget(mBgColorButton);
    mMainLayout->addLayout(mBgColorLayout);

    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mButtonsLayout = new QHBoxLayout();
    mMainLayout->addLayout(mButtonsLayout);
    mButtonsLayout->addWidget(mOkButton);
    mButtonsLayout->addWidget(mCancelButton);

    connect(mOkButton, SIGNAL(released()),
            this, SLOT(accept()));
    connect(mCancelButton, SIGNAL(released()),
            this, SLOT(reject()));
}

int CanvasSettingsDialog::getCanvasWidth() {
    return mWidthSpinBox->value();
}

int CanvasSettingsDialog::getCanvasHeight() {
    return mHeightSpinBox->value();
}

QString CanvasSettingsDialog::getCanvasName() {
    return mNameEdit->text();
}

int CanvasSettingsDialog::getCanvasFrameCount() {
    return mFrameCountSpinBox->value();
}

qreal CanvasSettingsDialog::getFps() {
    return mFPSSpinBox->value();
}

void CanvasSettingsDialog::applySettingsToCanvas(Canvas *canvas) {
    canvas->setName(getCanvasName());
    canvas->setCanvasSize(getCanvasWidth(), getCanvasHeight());
    canvas->setFps(getFps());
    canvas->setMaxFrame(getCanvasFrameCount());
}
