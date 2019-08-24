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

#include "spatialdisplacepatheffect.h"
#include "Animators/intanimator.h"
#include "randomgrid.h"
#include "Properties/boolproperty.h"
#include "pointhelpers.h"

SpatialDisplacePathEffect::SpatialDisplacePathEffect() :
    PathEffect("spatial displace effect",
               PathEffectType::SPATIAL_DISPLACE) {
    mSegLength = enve::make_shared<QrealAnimator>("segment length");
    mMaxDev = enve::make_shared<QrealAnimator>("max deviation");
    mSmoothness = QrealAnimator::sCreate0to1Animator("smoothness");
    mLengthBased = enve::make_shared<BoolPropertyContainer>("length inc");
    mSeed = enve::make_shared<RandomGrid>();

    mSegLength->setValueRange(1, 1000);
    mSegLength->setCurrentBaseValue(20);

    mMaxDev->setValueRange(0, 1000);
    mMaxDev->setCurrentBaseValue(20);

    ca_addChild(mSeed);
    ca_addChild(mMaxDev);
    ca_addChild(mLengthBased);

    mLengthBased->ca_addChild(mSegLength);
    mLengthBased->ca_addChild(mSmoothness);
    mLengthBased->setValue(false);
}

void SpatialDisplacePathEffect::apply(const qreal relFrame,
                                      const SkPath &src,
                                      SkPath * const dst) {
    const qreal baseSeed = mSeed->getBaseSeed(relFrame);
    const qreal gridSize = mSeed->getGridSize(relFrame);
    const qreal qMaxDev = mMaxDev->getEffectiveValue(relFrame);
    if(isZero4Dec(qMaxDev)) {
        *dst = src;
        return;
    }
    const qreal qSegLen = mSegLength->getEffectiveValue(relFrame);
    const qreal qSmooth = mSmoothness->getEffectiveValue(relFrame);    

    dst->reset();

    const float maxDev = toSkScalar(qMaxDev);
    const float segLen = toSkScalar(qSegLen);
    const float smooth = toSkScalar(qSmooth);

    if(mLengthBased->getValue()) {
        gSpatialDisplaceFilterPath(baseSeed, gridSize,
                                   dst, src, maxDev, segLen, smooth);
    } else {
        gSpatialDisplaceFilterPath(baseSeed, gridSize,
                                   dst, src, maxDev);
    }
}
