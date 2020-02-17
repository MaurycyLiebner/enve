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

#include "timelinesettingswidget.h"
#include "Private/esettings.h"

TimelineSettingsWidget::TimelineSettingsWidget(QWidget *parent) :
    SettingsWidget(parent) {
    mHighlightRowCheck = new QCheckBox("Highlight row under mouse", this);
    addWidget(mHighlightRowCheck);
}

void TimelineSettingsWidget::applySettings() {
    eSettings& sett = *eSettings::sInstance;
    sett.fTimelineHighlightRow = mHighlightRowCheck->isChecked();
}

void TimelineSettingsWidget::updateSettings() {
    eSettings& sett = *eSettings::sInstance;
    mHighlightRowCheck->setChecked(sett.fTimelineHighlightRow);
}
