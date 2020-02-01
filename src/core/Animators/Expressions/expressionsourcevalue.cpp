#include "expressionsourcevalue.h"
#include "Animators/qrealanimator.h"

ExpressionSourceValue::ExpressionSourceValue(
        QrealAnimator * const parent) :
    ExpressionSourceBase(parent) {
    setSource(parent);
}

ExpressionValue::sptr ExpressionSourceValue::sCreate(
        QrealAnimator * const parent) {
    return sptr(new ExpressionSourceValue(parent));
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
