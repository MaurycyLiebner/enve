#include "customfpsdialog.h"
#include <QPushButton>

CustomFpsDialog::CustomFpsDialog() {
    mMainLayout = new QVBoxLayout(this);
    setLayout(mMainLayout);

    mCheckBox = new QCheckBox("Enabled", this);

    mFpsLayout = new QHBoxLayout();
    mFpsLabel = new QLabel("Fps", this);
    mSpinBox = new QDoubleSpinBox(this);
    mSpinBox->setRange(1., 100.);

    mFpsLayout->addWidget(mFpsLabel);
    mFpsLayout->addWidget(mSpinBox);

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

    mMainLayout->addWidget(mCheckBox);
    mMainLayout->addLayout(mFpsLayout);
    mMainLayout->addLayout(mButtonsLayout);
}
