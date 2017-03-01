#include "newcanvasdialog.h"

NewCanvasDialog::NewCanvasDialog(const QString &defName, QWidget *parent) :
    QDialog(parent) {
    setWindowTitle("New Canvas");
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mNameLayout = new QHBoxLayout();
    mNameEditLabel = new QLabel("Name: ", this);
    mNameEdit = new QLineEdit(defName, this);
    mNameLayout->addWidget(mNameEditLabel);
    mNameLayout->addWidget(mNameEdit);
    mMainLayout->addLayout(mNameLayout);

    mWidthLabel = new QLabel("Width:", this);
    mWidthSpinBox = new QSpinBox(this);
    mWidthSpinBox->setRange(1, 9999);
    mWidthSpinBox->setValue(1920);

    mHeightLabel = new QLabel("Height:", this);
    mHeightSpinBox = new QSpinBox(this);
    mHeightSpinBox->setRange(1, 9999);
    mHeightSpinBox->setValue(1080);

    mSizeLayout = new QHBoxLayout();
    mSizeLayout->addWidget(mWidthLabel);
    mSizeLayout->addWidget(mWidthSpinBox);
    mSizeLayout->addWidget(mHeightLabel);
    mSizeLayout->addWidget(mHeightSpinBox);
    mMainLayout->addLayout(mSizeLayout);

    mFrameCountLabel = new QLabel("Frame Count:", this);
    mFrameCountSpinBox = new QSpinBox(this);
    mFrameCountSpinBox->setRange(1, 9999);
    mFrameCountSpinBox->setValue(200);

    mFrameCountLayout = new QHBoxLayout();
    mFrameCountLayout->addWidget(mFrameCountLabel);
    mFrameCountLayout->addWidget(mFrameCountSpinBox);
    mMainLayout->addLayout(mFrameCountLayout);

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

int NewCanvasDialog::getCanvasWidth() {
    return mWidthSpinBox->value();
}

int NewCanvasDialog::getCanvasHeight() {
    return mHeightSpinBox->value();
}

QString NewCanvasDialog::getCanvasName() {
    return mNameEdit->text();
}

int NewCanvasDialog::getCanvasFrameCount() {
    return mFrameCountSpinBox->value();
}
