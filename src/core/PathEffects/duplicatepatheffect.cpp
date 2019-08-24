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

#include "duplicatepatheffect.h"
#include "Animators/qpointfanimator.h"
#include "Animators/intanimator.h"

DuplicatePathEffect::DuplicatePathEffect() :
    PathEffect("duplicate effect", PathEffectType::DUPLICATE) {
    mTranslation = enve::make_shared<QPointFAnimator>("translation");
    mTranslation->setBaseValue(QPointF(10, 10));
    ca_addChild(mTranslation);

    mCount = enve::make_shared<IntAnimator>(1, 0, 25, 1, "count");
    ca_addChild(mCount);
}

void DuplicatePathEffect::apply(const qreal relFrame,
                                const SkPath &src,
                                SkPath * const dst) {
    *dst = src;
    const qreal qX = mTranslation->getEffectiveXValue(relFrame);
    const qreal qY = mTranslation->getEffectiveYValue(relFrame);
    const int count = mCount->getEffectiveIntValue(relFrame);
    for(int i = 0; i < count; i++)
        dst->addPath(src, toSkScalar(qX), toSkScalar(qY));
}
