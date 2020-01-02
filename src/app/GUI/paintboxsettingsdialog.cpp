// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "paintboxsettingsdialog.h"
#include <QPushButton>

PaintBoxSettingsDialog::PaintBoxSettingsDialog(QWidget *parent) :
    QDialog(parent) {
    mMainLayout = new QVBoxLayout();
    setLayout(mMainLayout);

    mFrameStepLayout = new QHBoxLayout();
    mOverlapFramesLayout = new QHBoxLayout();

    mMainLayout->addLayout(mFrameStepLayout);
    mMainLayout->addLayout(mOverlapFramesLayout);

    mFrameStepLabel = new QLabel("Frame step: ", this);
    mOverlapFramesLabel = new QLabel("Overlap frames: ", this);
    mFrameStepSpin = new QSpinBox(this);
    mFrameStepSpin->setRange(1, 99);
    mOverlapFramesSpin = new QSpinBox(this);
    mOverlapFramesSpin->setRange(0, 99);

    mFrameStepLayout->addWidget(mFrameStepLabel);
    mOverlapFramesLayout->addWidget(mOverlapFramesLabel);

    mFrameStepLayout->addWidget(mFrameStepSpin);
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
}

int PaintBoxSettingsDialog::getFrameStep() {
    return mFrameStepSpin->value();
}

int PaintBoxSettingsDialog::getOverlapFrames() {
    return mOverlapFramesSpin->value();
}
