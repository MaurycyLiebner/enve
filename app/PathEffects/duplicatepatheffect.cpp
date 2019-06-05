#include "duplicatepatheffect.h"
#include "Animators/qpointfanimator.h"
#include "Animators/intanimator.h"

DuplicatePathEffect::DuplicatePathEffect(const bool outlinePathEffect) :
    PathEffect("duplicate effect", DUPLICATE_PATH_EFFECT, outlinePathEffect) {
    mTranslation = SPtrCreate(QPointFAnimator)("translation");
    mTranslation->setBaseValue(QPointF(10, 10));
    ca_addChildAnimator(mTranslation);

    mCount = SPtrCreate(IntAnimator)(1, 0, 25, 1, "count");
    ca_addChildAnimator(mCount);
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
