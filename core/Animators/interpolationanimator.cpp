#include "interpolationanimator.h"
#include "graphkey.h"
#include "qrealpoint.h"

InterpolationAnimator::InterpolationAnimator(const QString &name) :
    GraphAnimator(name) {}

qreal InterpolationAnimator::getInterpolatedFrameAtRelFrameF(
        const qreal &frame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId, frame) ) {
        if(nextId == prevId) {
            return GetAsGK(anim_mKeys.at(nextId))->getRelFrame();
        } else {
            GraphKey *prevKey = GetAsGK(anim_mKeys.at(prevId));
            GraphKey *nextKey = GetAsGK(anim_mKeys.at(nextId));
            return getInterpolatedFrameAtRelFrameF(
                        frame, prevKey, nextKey);
        }
    }
    return frame;
}

void InterpolationAnimator::getValueConstraints(
        GraphKey *key, const QrealPointType &type,
        qreal &minValue, qreal &maxValue) const {
    if(type == QrealPointType::KEY_POINT) {
        minValue = key->getRelFrame();
        maxValue = minValue;
        //getFrameConstraints(key, type, minValue, maxValue);
    } else {
        minValue = DBL_MIN;
        maxValue = DBL_MAX;
    }
}

qreal InterpolationAnimator::getInterpolatedFrameAtRelFrameF(
        const qreal &frame, GraphKey *prevKey, GraphKey *nextKey) const {
    qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                         prevKey->getEndFrame(),
                         nextKey->getStartFrame(),
                         qreal(nextKey->getRelFrame())};
    qreal t = gTFromX(seg, frame);
    qreal p0y = prevKey->getValueForGraph();
    qreal p1y = prevKey->getEndValue();
    qreal p2y = nextKey->getStartValue();
    qreal p3y = nextKey->getValueForGraph();
    return gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
}
