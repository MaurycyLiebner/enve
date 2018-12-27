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

    connect(mOkButton, SIGNAL(pressed()),
            this, SLOT(ok()) );
    connect(mCancelButton, SIGNAL(pressed()),
            this, SLOT(cancel()) );
    connect(mFrameSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setFrame(double)) );
    connect(mValueSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setValue(double)) );
}

void QrealPointValueDialog::setValue(double value)
{
    mPoint->setValue(value);
    emit repaintSignal();
}

void QrealPointValueDialog::setFrame(double frame)
{
    mPoint->setFrame(frame);
    emit repaintSignal();
}

void QrealPointValueDialog::cancel()
{
    resetValue();
    close();
}

void QrealPointValueDialog::ok()
{
    close();
}

void QrealPointValueDialog::resetValue()
{
    mValueSpinBox->setValue(mSavedValue);
    mFrameSpinBox->setValue(mSavedFrame);
    setValue(mSavedValue);
    setFrame(mSavedFrame);
    emit repaintSignal();
}
