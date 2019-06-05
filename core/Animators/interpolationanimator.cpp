#include "interpolationanimator.h"
#include "graphkey.h"
#include "qrealpoint.h"

InterpolationAnimator::InterpolationAnimator(const QString &name) :
    GraphAnimator(name) {}

qreal InterpolationAnimator::getInterpolatedFrameAtRelFrame(
        const qreal frame) const {
    if(anim_mKeys.isEmpty()) return frame;
    const auto pn = anim_getPrevAndNextKeyIdF(frame);
    const int prevId = pn.first;
    const int nextId = pn.second;

    const bool adjKeys = nextId - prevId == 1;
    const auto keyAtRelFrame = adjKeys ? nullptr :
                               anim_getKeyAtIndex(prevId + 1);
    if(keyAtRelFrame) return frame;
    const auto prevKey = anim_getKeyAtIndex<GraphKey>(prevId);
    const auto nextKey = anim_getKeyAtIndex<GraphKey>(nextId);
    if(!prevKey || !nextKey) {
        return frame;
    } else { // if(prevKey && nextKey) {
        const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                                  prevKey->getEndFrame(),
                                  nextKey->getStartFrame(),
                                  qreal(nextKey->getRelFrame())};
        const qreal t = gTFromX(seg, frame);
        const qreal p0y = prevKey->getValueForGraph();
        const qreal p1y = prevKey->getEndValue();
        const qreal p2y = nextKey->getStartValue();
        const qreal p3y = nextKey->getValueForGraph();
        return gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
    }
}

void InterpolationAnimator::graph_getValueConstraints(
        GraphKey *key, const QrealPointType &type,
        qreal &minValue, qreal &maxValue) const {
    if(type == QrealPointType::KEY_POINT) {
        minValue = key->getRelFrame();
        maxValue = minValue;
        //getFrameConstraints(key, type, minValue, maxValue);
    } else {
        minValue = -DBL_MAX;
        maxValue = DBL_MAX;
    }
}
