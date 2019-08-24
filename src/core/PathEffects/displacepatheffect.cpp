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

#include "displacepatheffect.h"
#include "Animators/intanimator.h"
#include "Properties/boolproperty.h"
#include "pointhelpers.h"

DisplacePathEffect::DisplacePathEffect() :
    PathEffect("displace effect", PathEffectType::DISPLACE) {
    mSegLength = enve::make_shared<QrealAnimator>("segment length");
    mMaxDev = enve::make_shared<QrealAnimator>("max deviation");
    mSmoothness = QrealAnimator::sCreate0to1Animator("smoothness");
    mLengthBased = enve::make_shared<BoolPropertyContainer>("length inc");
    mSeed = enve::make_shared<QrealAnimator>(qrand() % 9999, 0, 9999, 1, "smooth seed");

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

std::pair<QPointF, QPointF> posAndTanToC1C2(const PosAndTan& prev,
                                            const PosAndTan& next,
                                            const qreal smooth) {
    const qreal thirdDist = 0.333*pointToLen(next.fPos - prev.fPos);
    const auto c1Vector = scalePointToNewLen(prev.fTan, thirdDist);
    const QPointF c1 = prev.fPos + c1Vector*smooth;
    const auto c2Vector = scalePointToNewLen(next.fTan, thirdDist);
    const QPointF c2 = next.fPos - c2Vector*smooth;
    return {c1, c2};
}

void perterb(PosAndTan& posAndTan, const qreal dev) {
    posAndTan.fTan = scalePointToNewLen(posAndTan.fTan, dev);
    const auto displ = gRotPt(posAndTan.fTan, 90);
    posAndTan.fPos += displ;
}

void DisplacePathEffect::apply(const qreal relFrame,
                               const SkPath &src,
                               SkPath * const dst) {
    const qreal baseSeed = mSeed->getEffectiveValue(relFrame);

    const qreal qMaxDev = mMaxDev->getEffectiveValue(relFrame);
    const qreal qSegLen = mSegLength->getEffectiveValue(relFrame);
    const qreal qSmooth = mSmoothness->getEffectiveValue(relFrame);    

    dst->reset();

    const float maxDev = toSkScalar(qMaxDev);
    const float segLen = toSkScalar(qSegLen);
    const float smooth = toSkScalar(qSmooth);

    if(mLengthBased->getValue()) {
        gAtomicDisplaceFilterPath(baseSeed, dst, src, maxDev, segLen, smooth);
    } else {
        gAtomicDisplaceFilterPath(baseSeed, dst, src, maxDev);
    }
}
