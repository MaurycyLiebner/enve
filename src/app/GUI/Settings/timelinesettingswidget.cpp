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

#include "GUI/coloranimatorbutton.h"

#include <QLabel>

TimelineSettingsWidget::TimelineSettingsWidget(QWidget *parent) :
    SettingsWidget(parent) {
    mAlternateRowCheck = new QCheckBox("Alternate row color", this);
    mAlternateRowColor = new ColorAnimatorButton(
                mSett.fTimelineAlternateRowColor);
    add2HWidgets(mAlternateRowCheck, mAlternateRowColor);

    mHighlightRowCheck = new QCheckBox("Highlight row under mouse", this);
    mHighlightRowColor = new ColorAnimatorButton(
                mSett.fTimelineHighlightRowColor);
    add2HWidgets(mHighlightRowCheck, mHighlightRowColor);

    addSeparator();

    mObjectKeyframeColor = new ColorAnimatorButton(
                               mSett.fObjectKeyframeColor);
    mPropertyGroupKeyframeColor = new ColorAnimatorButton(
                               mSett.fPropertyGroupKeyframeColor);
    mPropertyKeyframeColor = new ColorAnimatorButton(
                               mSett.fPropertyKeyframeColor);
    mSelectedKeyframeColor = new ColorAnimatorButton(
                               mSett.fSelectedKeyframeColor);

    add2HWidgets(new QLabel("Object keyframe color"),
                 mObjectKeyframeColor);
    add2HWidgets(new QLabel("Property group keyframe color"),
                 mPropertyGroupKeyframeColor);
    add2HWidgets(new QLabel("Property keyframe color"),
                 mPropertyKeyframeColor);
    add2HWidgets(new QLabel("Selected keyframe color"),
                 mSelectedKeyframeColor);
}

void TimelineSettingsWidget::applySettings() {
    mSett.fTimelineAlternateRow = mAlternateRowCheck->isChecked();
    mSett.fTimelineAlternateRowColor = mAlternateRowColor->color();

    mSett.fTimelineHighlightRow = mHighlightRowCheck->isChecked();
    mSett.fTimelineHighlightRowColor = mHighlightRowColor->color();

    mSett.fObjectKeyframeColor = mObjectKeyframeColor->color();
    mSett.fPropertyGroupKeyframeColor = mPropertyGroupKeyframeColor->color();
    mSett.fPropertyKeyframeColor = mPropertyKeyframeColor->color();
    mSett.fSelectedKeyframeColor = mSelectedKeyframeColor->color();
}

void TimelineSettingsWidget::updateSettings() {
    mAlternateRowCheck->setChecked(mSett.fTimelineAlternateRow);
    mAlternateRowColor->setColor(mSett.fTimelineAlternateRowColor);

    mHighlightRowCheck->setChecked(mSett.fTimelineHighlightRow);
    mHighlightRowColor->setColor(mSett.fTimelineHighlightRowColor);

    mObjectKeyframeColor->setColor(mSett.fObjectKeyframeColor);
    mPropertyGroupKeyframeColor->setColor(mSett.fPropertyGroupKeyframeColor);
    mPropertyKeyframeColor->setColor(mSett.fPropertyKeyframeColor);
    mSelectedKeyframeColor->setColor(mSett.fSelectedKeyframeColor);
}
