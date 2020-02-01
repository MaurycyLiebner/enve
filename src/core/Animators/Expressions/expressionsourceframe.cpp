#include "expressionsourceframe.h"
#include "Animators/qrealanimator.h"

ExpressionSourceFrame::ExpressionSourceFrame(
        QrealAnimator * const parent) :
    ExpressionSourceBase(parent) {
    setSource(parent);
}

ExpressionValue::sptr ExpressionSourceFrame::sCreate(
        QrealAnimator * const parent) {
    return sptr(new ExpressionSourceFrame(parent));
}

qreal ExpressionSourceFrame::calculateValue(const qreal relFrame) const {
    return relFrame;
}

FrameRange ExpressionSourceFrame::identicalRange(const qreal relFrame) const {
    const auto src = source();
    if(!src) return FrameRange::EMINMAX;
    return {qFloor(relFrame), qCeil(relFrame)};
}
