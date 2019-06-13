#include "newcanvasdialog.h"
#include "canvas.h"
#include "GUI/BoxesList/coloranimatorbutton.h"

CanvasSettingsDialog::CanvasSettingsDialog(Canvas * const canvas,
                                           QWidget * const parent) :
    CanvasSettingsDialog(canvas->getName(),
                         canvas->getCanvasWidth(),
                         canvas->getCanvasHeight(),
                         canvas->getMaxFrame(),
                         canvas->getFps(),
                         canvas->getBgColorAnimator(),
                         parent) {
    mTargetCanvas = canvas;
}

CanvasSettingsDialog::CanvasSettingsDialog(const QString &defName,
                                           QWidget * const parent) :
    CanvasSettingsDialog(defName, 1920, 1080, 200, 24., nullptr, parent) {}

CanvasSettingsDialog::CanvasSettingsDialog(const QString &name,
                                           const int width,
                                           const int height,
                                           const int frameCount,
                                           const qreal fps,
                                           ColorAnimator * const bg,
                                           QWidget * const parent) :
    QDialog(parent) {
    setWindowTitle("Canvas Settings");
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mNameLayout = new QHBoxLayout();
    mNameEditLabel = new QLabel("Name: ", this);
    mNameEdit = new QLineEdit(name, this);
    mNameLayout->addWidget(mNameEditLabel);
    mNameLayout->addWidget(mNameEdit);
    mMainLayout->addLayout(mNameLayout);

    mWidthLabel = new QLabel("Width:", this);
    mWidthSpinBox = new QSpinBox(this);
    mWidthSpinBox->setRange(1, 9999);
    mWidthSpinBox->setValue(width);

    mHeightLabel = new QLabel("Height:", this);
    mHeightSpinBox = new QSpinBox(this);
    mHeightSpinBox->setRange(1, 9999);
    mHeightSpinBox->setValue(height);

    mSizeLayout = new QHBoxLayout();
    mSizeLayout->addWidget(mWidthLabel);
    mSizeLayout->addWidget(mWidthSpinBox);
    mSizeLayout->addWidget(mHeightLabel);
    mSizeLayout->addWidget(mHeightSpinBox);
    mMainLayout->addLayout(mSizeLayout);

    mFrameCountLabel = new QLabel("Frame Count:", this);
    mFrameCountSpinBox = new QSpinBox(this);
    mFrameCountSpinBox->setRange(1, 999999);
    mFrameCountSpinBox->setValue(frameCount);

    mFrameCountLayout = new QHBoxLayout();
    mFrameCountLayout->addWidget(mFrameCountLabel);
    mFrameCountLayout->addWidget(mFrameCountSpinBox);
    mMainLayout->addLayout(mFrameCountLayout);

    mFPSLabel = new QLabel("Fps:", this);
    mFPSSpinBox = new QDoubleSpinBox(this);
    mFPSSpinBox->setRange(1, 300);
    mFPSSpinBox->setValue(fps);

    mFPSLayout = new QHBoxLayout();
    mFPSLayout->addWidget(mFPSLabel);
    mFPSLayout->addWidget(mFPSSpinBox);
    mMainLayout->addLayout(mFPSLayout);

    mBgColorLabel = new QLabel("Backgroud:", this);
    mBgColorButton = new ColorAnimatorButton(bg, this);
    if(!bg) mBgColorButton->setColor(Qt::white);

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

    connect(mOkButton, &QPushButton::released,
            this, &CanvasSettingsDialog::accept);
    connect(mOkButton, &QPushButton::released,
            this, &CanvasSettingsDialog::accept);
    connect(mCancelButton, &QPushButton::released,
            this, &CanvasSettingsDialog::reject);
    connect(this, &QDialog::rejected, this, &QDialog::close);
}

int CanvasSettingsDialog::getCanvasWidth() const {
    return mWidthSpinBox->value();
}

int CanvasSettingsDialog::getCanvasHeight() const {
    return mHeightSpinBox->value();
}

QString CanvasSettingsDialog::getCanvasName() const {
    return mNameEdit->text();
}

int CanvasSettingsDialog::getCanvasFrameCount() const {
    return mFrameCountSpinBox->value();
}

qreal CanvasSettingsDialog::getFps() const {
    return mFPSSpinBox->value();
}

void CanvasSettingsDialog::applySettingsToCanvas(Canvas * const canvas) const {
    if(!canvas) return;
    canvas->setName(getCanvasName());
    canvas->setCanvasSize(getCanvasWidth(), getCanvasHeight());
    canvas->setFps(getFps());
    canvas->setMaxFrame(getCanvasFrameCount());
    if(canvas != mTargetCanvas) {
        canvas->getBgColorAnimator()->setColor(mBgColorButton->color());
    }
}
