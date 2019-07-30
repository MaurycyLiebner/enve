#include "spatialdisplacepatheffect.h"
#include "Animators/intanimator.h"
#include "randomgrid.h"
#include "Properties/boolproperty.h"
#include "pointhelpers.h"

SpatialDisplacePathEffect::SpatialDisplacePathEffect() :
    PathEffect("spatial displace effect",
               PathEffectType::SPATIAL_DISPLACE) {
    mSegLength = SPtrCreate(QrealAnimator)("segment length");
    mMaxDev = SPtrCreate(QrealAnimator)("max deviation");
    mSmoothness = QrealAnimator::sCreate0to1Animator("smoothness");
    mLengthBased = SPtrCreate(BoolPropertyContainer)("length inc");
    mSeed = SPtrCreate(RandomGrid)();

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
