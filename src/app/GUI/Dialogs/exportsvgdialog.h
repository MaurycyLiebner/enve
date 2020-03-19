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

#ifndef EXPORTSVGDIALOG_H
#define EXPORTSVGDIALOG_H

#include <QDialog>

#include <QSpinBox>
#include <QCheckBox>

class SceneChooser;

class ExportSvgDialog : public QDialog {
public:
    ExportSvgDialog(const QString& path,
                    QWidget* const parent = nullptr);

private:
    SceneChooser* mScene;

    QSpinBox* mFirstFrame;
    QSpinBox* mLastFrame;

    QCheckBox* mBackground;
    QCheckBox* mFixedSize;
    QCheckBox* mLoop;

    const QString mPath;
};

#endif // EXPORTSVGDIALOG_H
