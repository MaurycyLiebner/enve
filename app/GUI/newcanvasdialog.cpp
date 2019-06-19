#include "newcanvasdialog.h"
#include "canvas.h"
#include "GUI/BoxesList/coloranimatorbutton.h"

CanvasSettingsDialog::CanvasSettingsDialog(Canvas * const canvas,
                                           QWidget * const parent) :
    CanvasSettingsDialog(canvas->getName(),
                         canvas->getCanvasWidth(),
                         canvas->getCanvasHeight(),
                         canvas->getFrameRange(),
                         canvas->getFps(),
                         canvas->getBgColorAnimator(),
                         parent) {
    mTargetCanvas = canvas;
}

CanvasSettingsDialog::CanvasSettingsDialog(const QString &defName,
                                           QWidget * const parent) :
    CanvasSettingsDialog(defName, 1920, 1080, {0, 200}, 24., nullptr, parent) {}

CanvasSettingsDialog::CanvasSettingsDialog(const QString &name,
                                           const int width,
                                           const int height,
                                           const FrameRange& range,
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

    mFrameRangeLabel = new QLabel("Frame Range:", this);
    mMinFrameSpin = new QSpinBox(this);
    mMinFrameSpin->setRange(0, 999999);
    mMinFrameSpin->setValue(range.fMin);

    mMaxFrameSpin = new QSpinBox(this);
    mMaxFrameSpin->setRange(0, 999999);
    mMaxFrameSpin->setValue(range.fMax);

    mFrameRangeLayout = new QHBoxLayout();
    mFrameRangeLayout->addWidget(mFrameRangeLabel);
    mFrameRangeLayout->addWidget(mMinFrameSpin);
    mFrameRangeLayout->addWidget(mMaxFrameSpin);

    mMainLayout->addLayout(mFrameRangeLayout);

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

FrameRange CanvasSettingsDialog::getFrameRange() const {
    FrameRange range = {mMinFrameSpin->value(), mMaxFrameSpin->value()};
    range.fixOrder();
    return range;
}

qreal CanvasSettingsDialog::getFps() const {
    return mFPSSpinBox->value();
}

void CanvasSettingsDialog::applySettingsToCanvas(Canvas * const canvas) const {
    if(!canvas) return;
    canvas->setName(getCanvasName());
    canvas->setCanvasSize(getCanvasWidth(), getCanvasHeight());
    canvas->setFps(getFps());
    canvas->setFrameRange(getFrameRange());
    if(canvas != mTargetCanvas) {
        canvas->getBgColorAnimator()->setColor(mBgColorButton->color());
    }
}

void CanvasSettingsDialog::sNewCanvasDialog(Document& document,
                                            QWidget * const parent) {
    const QString defName = "Scene " +
            QString::number(document.fScenes.count());

    const auto dialog = new CanvasSettingsDialog(defName, parent);
    const auto docPtr = &document;
    connect(dialog, &QDialog::accepted, dialog, [dialog, docPtr]() {
        const auto newCanvas = docPtr->createNewScene();
        dialog->applySettingsToCanvas(newCanvas);
        dialog->close();
    });

    dialog->show();
}
