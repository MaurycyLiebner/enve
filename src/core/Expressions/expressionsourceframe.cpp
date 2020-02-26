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

#include "expressionsourceframe.h"
#include "Animators/qrealanimator.h"

ExpressionSourceFrame::ExpressionSourceFrame(
        QrealAnimator * const parent) :
    ExpressionSourceBase(parent) {
    setSource(parent);
}

ExpressionValue::sptr ExpressionSourceFrame::sCreate(
        QrealAnimator * const parent) {
    const auto result = new ExpressionSourceFrame(parent);
    result->updateValue();
    return sptr(result);
}

qreal ExpressionSourceFrame::calculateValue(const qreal relFrame) const {
    return relFrame;
}

FrameRange ExpressionSourceFrame::identicalRange(const qreal relFrame) const {
    const auto src = source();
    if(!src) return FrameRange::EMINMAX;
    return {qFloor(relFrame), qCeil(relFrame)};
}
