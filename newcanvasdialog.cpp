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

    mWidthSpinBox = new QSpinBox(this);
    mWidthSpinBox->setValue(1920);
    mHeightSpinBox = new QSpinBox(this);
    mHeightSpinBox->setValue(1080);
    mSizeLayout = new QHBoxLayout();
    mSizeLayout->addWidget(mWidthSpinBox);
    mSizeLayout->addWidget(mHeightSpinBox);
    mMainLayout->addLayout(mSizeLayout);

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
