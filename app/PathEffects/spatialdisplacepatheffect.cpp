#include "spatialdisplacepatheffect.h"
#include "Animators/intanimator.h"
#include "randomgrid.h"
#include "Properties/boolproperty.h"
#include "pointhelpers.h"

SpatialDisplacePathEffect::SpatialDisplacePathEffect() :
    PathEffect("spatial displace effect",
               SPATIAL_DISPLACE_PATH_EFFECT) {
    mSegLength = SPtrCreate(QrealAnimator)("segment length");
    mMaxDev = SPtrCreate(QrealAnimator)("max deviation");
    mSmoothness = QrealAnimator::create0to1Animator("smoothness");
    mLengthBased = SPtrCreate(BoolPropertyContainer)("length inc");
    mSeed = SPtrCreate(RandomGrid)();

    mSegLength->setValueRange(1, 1000);
    mSegLength->setCurrentBaseValue(20);

    mMaxDev->setValueRange(0, 1000);
    mMaxDev->setCurrentBaseValue(20);

    ca_addChildAnimator(mSeed);
    ca_addChildAnimator(mMaxDev);
    ca_addChildAnimator(mLengthBased);

    mLengthBased->ca_addChildAnimator(mSegLength);
    mLengthBased->ca_addChildAnimator(mSmoothness);
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

    const SkScalar maxDev = toSkScalar(qMaxDev);
    const SkScalar segLen = toSkScalar(qSegLen);
    const SkScalar smooth = toSkScalar(qSmooth);

    if(mLengthBased->getValue()) {
        gSpatialDisplaceFilterPath(baseSeed, gridSize,
                                   dst, src, maxDev, segLen, smooth);
    } else {
        gSpatialDisplaceFilterPath(baseSeed, gridSize,
                                   dst, src, maxDev);
    }
}

void SpatialDisplacePathEffect::writeProperty(QIODevice * const dst) const {
    PathEffect::writeProperty(dst);
    mMaxDev->writeProperty(dst);
    mLengthBased->writeProperty(dst);
    mSeed->writeProperty(dst);
    mSegLength->writeProperty(dst);
    mSmoothness->writeProperty(dst);
}

void SpatialDisplacePathEffect::readProperty(QIODevice * const src) {
    PathEffect::readProperty(src);
    mMaxDev->readProperty(src);
    mLengthBased->readProperty(src);
    mSeed->readProperty(src);
    mSegLength->readProperty(src);
    mSmoothness->readProperty(src);
}
