// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "durationrectsettingsdialog.h"
#include "twocolumnlayout.h"


DurationRectSettingsDialog::DurationRectSettingsDialog(
        const int shift,
        const int minFrame,
        const int maxFrame,
        QWidget *parent) :
    QDialog(parent) {
    mMainLayout = new QVBoxLayout(this);
    mTwoColumnLayout = new TwoColumnLayout();

    mShiftLabel = new QLabel("Frame shift:");
    mShiftSpinBox = new QSpinBox(this);
    mShiftSpinBox->setRange(-99999, 99999);
    mShiftSpinBox->setValue(shift);
    mTwoColumnLayout->addPair(mShiftLabel, mShiftSpinBox);

    mMinFrameLabel = new QLabel("Min frame:");
    mMinFrameSpinBox = new QSpinBox(this);
    mMinFrameSpinBox->setRange(-99999, maxFrame);
    mMinFrameSpinBox->setValue(minFrame);
    mTwoColumnLayout->addPair(mMinFrameLabel, mMinFrameSpinBox);

    mMaxFrameLabel = new QLabel("Max frame:");
    mMaxFrameSpinBox = new QSpinBox(this);
    mMaxFrameSpinBox->setRange(minFrame, 99999);
    mMaxFrameSpinBox->setValue(maxFrame);
    mTwoColumnLayout->addPair(mMaxFrameLabel, mMaxFrameSpinBox);

    connect(mMinFrameSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            mMaxFrameSpinBox, &QSpinBox::setMinimum);
    connect(mMaxFrameSpinBox, qOverload<int>(&QSpinBox::valueChanged),
            mMinFrameSpinBox, &QSpinBox::setMaximum);

    mButtonsLayout = new QHBoxLayout();
    mOkButton = new QPushButton("Ok", this);
    mCancelButton = new QPushButton("Cancel", this);
    mButtonsLayout->addWidget(mOkButton);
    mButtonsLayout->addWidget(mCancelButton);
    mButtonsLayout->setAlignment(Qt::AlignRight);
    connect(mOkButton, &QPushButton::pressed, this, &QDialog::accept);
    connect(mCancelButton, &QPushButton::pressed, this, &QDialog::reject);

    mMainLayout->addLayout(mTwoColumnLayout);
    mMainLayout->addLayout(mButtonsLayout);
    setLayout(mMainLayout);
}
