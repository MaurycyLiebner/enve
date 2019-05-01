#include "qrealpointvaluedialog.h"
#include "Animators/qrealpoint.h"

QrealPointValueDialog::QrealPointValueDialog(QrealPoint *point,
                                             QWidget *parent) :
    QDialog(parent) {
    setWindowTitle("Edit key");
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mFrameSpinBox = new QDoubleSpinBox(this);
    mValueSpinBox = new QDoubleSpinBox(this);
    mSpinLayout = new QHBoxLayout();
    mSpinLayout->addWidget(mFrameSpinBox);
    mSpinLayout->addWidget(mValueSpinBox);
    mFrameSpinBox->setRange(0, 10000);
    if(point->isKeyPoint()) {
        mFrameSpinBox->setDecimals(0);
    }
    mValueSpinBox->setRange(-10000, 10000);

    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mButtonsLayout = new QHBoxLayout();
    mButtonsLayout->addWidget(mOkButton);
    mButtonsLayout->addWidget(mCancelButton);

    mMainLayout->addLayout(mSpinLayout);
    mMainLayout->addLayout(mButtonsLayout);

    mPoint = point;
    mSavedValue = mPoint->getValue();
    mSavedFrame = mPoint->getFrame();
    resetValue();

    connect(mOkButton, &QPushButton::pressed,
            this, &QrealPointValueDialog::ok);
    connect(mCancelButton, &QPushButton::pressed,
            this, &QrealPointValueDialog::cancel);
    connect(mFrameSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &QrealPointValueDialog::setFrame);
    connect(mValueSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged),
            this, &QrealPointValueDialog::setValue);
}

void QrealPointValueDialog::setValue(double value) {
    mPoint->setValue(value);
    emit repaintSignal();
}

void QrealPointValueDialog::setFrame(double frame) {
    mPoint->setFrame(frame);
    emit repaintSignal();
}

void QrealPointValueDialog::cancel() {
    resetValue();
    close();
}

void QrealPointValueDialog::ok() {
    close();
}

void QrealPointValueDialog::resetValue() {
    mValueSpinBox->setValue(mSavedValue);
    mFrameSpinBox->setValue(mSavedFrame);
    setValue(mSavedValue);
    setFrame(mSavedFrame);
    emit repaintSignal();
}
