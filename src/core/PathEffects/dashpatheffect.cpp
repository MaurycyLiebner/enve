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

#include "dashpatheffect.h"
#include "Animators/qrealanimator.h"

DashPathEffect::DashPathEffect() :
    PathEffect("dash effect", PathEffectType::DASH) {
    mSize = enve::make_shared<QrealAnimator>("size");
    mSize->setValueRange(0.1, 9999.999);
    mSize->setCurrentBaseValue(5);

    ca_addChild(mSize);

    setPropertyForGUI(mSize.get());
}

void DashPathEffect::apply(const qreal relFrame,
                           const SkPath &src,
                           SkPath * const dst) {
    const auto width = toSkScalar(mSize->getEffectiveValue(relFrame));
    const float intervals[] = { width, width };
    SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
    SkRect cullRec = src.getBounds();
    dst->reset();
    SkDashPathEffect::Make(intervals, 2, 0.f)->filterPath(dst, src, &rec, &cullRec);
}
