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

#include "brushsettingsanimator.h"
#include "Paint/brushcontexedwrapper.h"

BrushSettingsAnimator::BrushSettingsAnimator() : StaticComplexAnimator("brush settings") {
    ca_addChild(mWidthCurve);
    ca_addChild(mPressureCurve);
    ca_addChild(mSpacingCurve);
    ca_addChild(mTimeCurve);

    mWidthCurve->setCurrentValue(qCubicSegment1D(1));
    mPressureCurve->setCurrentValue(qCubicSegment1D(1));
    mSpacingCurve->setCurrentValue(qCubicSegment1D(1));
    mTimeCurve->setCurrentValue(qCubicSegment1D(1));
}

void BrushSettingsAnimator::writeProperty(eWriteStream& dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst << (mBrush ? mBrush->getCollectionName() : "");
    dst << (mBrush ? mBrush->getBrushName() : "");
}

void BrushSettingsAnimator::readProperty(eReadStream& src) {
    StaticComplexAnimator::readProperty(src);
    QString brushCollection; src >> brushCollection;
    QString brushName; src >> brushName;
    mBrush = BrushCollectionData::sGetBrush(brushCollection, brushName);
}
