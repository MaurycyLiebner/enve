#include "duplicatepatheffect.h"

DuplicatePathEffect::DuplicatePathEffect(const bool &outlinePathEffect) :
    PathEffect("duplicate effect", DUPLICATE_PATH_EFFECT, outlinePathEffect) {
    mTranslation = SPtrCreate(QPointFAnimator)("translation");
    mTranslation->setCurrentPointValue(QPointF(10., 10.));

    ca_addChildAnimator(mTranslation);
}

void DuplicatePathEffect::filterPathForRelFrameF(const qreal &relFrame,
                                                 const SkPath &src,
                                                 SkPath *dst,
                                                 const bool &) {
    *dst = src;
    dst->addPath(src,
                 mTranslation->getEffectiveXValueAtRelFrame(relFrame),
                 mTranslation->getEffectiveYValueAtRelFrame(relFrame));
}
