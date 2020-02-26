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

#include "expressionsourcevalue.h"
#include "Animators/qrealanimator.h"

ExpressionSourceValue::ExpressionSourceValue(
        QrealAnimator * const parent) :
    ExpressionSourceBase(parent) {
    auto& conn = setSource(parent);
    conn << connect(parent, &Property::prp_absFrameRangeChanged,
                    this, [this, parent](const FrameRange& absRange) {
        const auto relRange = parent->prp_absRangeToRelRange(absRange);
        const bool currentFrameAffected = relRange.inRange(relFrame());
        if(currentFrameAffected) updateValue();
    });
}

ExpressionValue::sptr ExpressionSourceValue::sCreate(
        QrealAnimator * const parent) {
    const auto result = new ExpressionSourceValue(parent);
    result->updateValue();
    return sptr(result);
}

qreal ExpressionSourceValue::calculateValue(const qreal relFrame) const {
    const auto src = source();
    if(!src) return 1;
    return src->getBaseValue(relFrame);
}

FrameRange ExpressionSourceValue::identicalRange(const qreal relFrame) const {
    const auto src = source();
    if(!src) return FrameRange::EMINMAX;
    return src->Animator::prp_getIdenticalRelRange(relFrame);
}
