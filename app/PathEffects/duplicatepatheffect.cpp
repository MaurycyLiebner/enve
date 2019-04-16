#include "duplicatepatheffect.h"
#include "Animators/qpointfanimator.h"

DuplicatePathEffect::DuplicatePathEffect(const bool &outlinePathEffect) :
    PathEffect("duplicate effect", DUPLICATE_PATH_EFFECT, outlinePathEffect) {
    mTranslation = SPtrCreate(QPointFAnimator)("translation");
    mTranslation->setBaseValue(QPointF(10, 10));
    ca_addChildAnimator(mTranslation);
}

void DuplicatePathEffect::apply(const qreal &relFrame,
                                const SkPath &src,
                                SkPath * const dst) {
    *dst = src;
    const qreal qX = mTranslation->getEffectiveXValue(relFrame);
    const qreal qY = mTranslation->getEffectiveYValue(relFrame);
    dst->addPath(src, toSkScalar(qX), toSkScalar(qY));
}
