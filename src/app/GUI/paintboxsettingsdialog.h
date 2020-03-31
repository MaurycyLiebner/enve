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

#ifndef PAINTBOXSETTINGSDIALOG_H
#define PAINTBOXSETTINGSDIALOG_H
#include <QDialog>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>

class PaintBoxSettingsDialog : public QDialog
{
public:
    PaintBoxSettingsDialog(QWidget *parent = nullptr);
    ~PaintBoxSettingsDialog();

    int getFirstFrame();
    int getFrameStep();
    int getFrameCount();
    int getOverlapFrames();

    void setOverlapFrames(const int overlapFrames) {
        mOverlapFramesSpin->setValue(overlapFrames);
    }

    void setFrameStep(const int frameStep) {
        mFrameStepSpin->setValue(frameStep);
    }
private:
    QVBoxLayout *mMainLayout = nullptr;

    QHBoxLayout *mFrameStepLayout = nullptr;
    QHBoxLayout *mOverlapFramesLayout = nullptr;

    QLabel *mFrameStepLabel = nullptr;
    QLabel *mOverlapFramesLabel = nullptr;

    QSpinBox *mFrameStepSpin = nullptr;
    QSpinBox *mOverlapFramesSpin = nullptr;

    QHBoxLayout *mButtonsLayout = nullptr;
    QPushButton *mAcceptButton = nullptr;
    QPushButton *mCancelButton = nullptr;
};

#endif // PAINTBOXSETTINGSDIALOG_H
