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

#ifndef DURATIONRECTSETTINGSDIALOG_H
#define DURATIONRECTSETTINGSDIALOG_H
#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include "Timeline/durationrectangle.h"
class QHBoxLayout;
class QVBoxLayout;
class TwoColumnLayout;

class DurationRectSettingsDialog : public QDialog {
public:
    DurationRectSettingsDialog(const int shift,
                               const int minFrame,
                               const int maxFrame,
                               QWidget *parent = nullptr);

    int getMinFrame() const {
        return mMinFrameSpinBox->value();
    }

    int getMaxFrame() const {
        return mMaxFrameSpinBox->value();
    }

    int getShift() const {
        return mShiftSpinBox->value();
    }
protected:
    QVBoxLayout *mMainLayout;
    TwoColumnLayout *mTwoColumnLayout;

    QLabel *mShiftLabel;
    QSpinBox *mShiftSpinBox;

    QLabel *mMinFrameLabel;
    QSpinBox *mMinFrameSpinBox;

    QLabel *mMaxFrameLabel;
    QSpinBox *mMaxFrameSpinBox;

    QLabel *mFirstAnimationFrameLabel;
    QSpinBox *mFirstAnimationFrameSpinBox = nullptr;

    QHBoxLayout *mButtonsLayout;
    QPushButton *mOkButton;
    QPushButton *mCancelButton;
};

#endif // DURATIONRECTSETTINGSDIALOG_H
