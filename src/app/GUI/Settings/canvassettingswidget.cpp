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

#include "canvassettingswidget.h"

#include "Private/esettings.h"

CanvasSettingsWidget::CanvasSettingsWidget(QWidget* const parent) :
    SettingsWidget(parent) {
    mRtlSupport = new QCheckBox("RTL language support", this);
    addWidget(mRtlSupport);
}

void CanvasSettingsWidget::applySettings() {
    mSett.fCanvasRtlSupport = mRtlSupport->isChecked();
}

void CanvasSettingsWidget::updateSettings() {
    mRtlSupport->setChecked(mSett.fCanvasRtlSupport);
}
