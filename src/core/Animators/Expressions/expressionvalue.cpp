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

#include "expressionvalue.h"

ExpressionValue::ExpressionValue(const bool needsBrackets) :
    mNeedsBrackets(needsBrackets) {}

bool ExpressionValue::setRelFrame(const qreal relFrame) {
    if(!setRelFrameValue(relFrame)) return false;
    return updateValue();
}

qreal ExpressionValue::value(const qreal relFrame) const {
    const auto idRange = identicalRange(mRelFrame);
    const bool inRange = idRange.inRange(relFrame);
    if(inRange) return mCurrentValue;
    return calculateValue(relFrame);
}

bool ExpressionValue::updateValue() {
    const qreal newValue = calculateValue(mRelFrame);
    if(isZero4Dec(mCurrentValue - newValue)) return false;
    mCurrentValue = newValue;
    emit currentValueChanged(mCurrentValue);
    return true;
}

bool ExpressionValue::setRelFrameValue(const qreal relFrame) {
    if(isZero4Dec(mRelFrame - relFrame)) return false;
    mRelFrame = relFrame;
    return true;
}
