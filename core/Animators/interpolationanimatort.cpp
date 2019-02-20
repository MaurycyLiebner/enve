#include "interpolationanimatort.h"
#include "qrealpoint.h"

qreal getInterpolatedFrameAtRelFrame(
        const qreal &frame, GraphKey * const prevKey,
        GraphKey * const nextKey) {
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

template <typename T, typename K>
qreal InterpolationAnimatorT<T, K>::getInterpolatedFrameAtRelFrame(
        const qreal &frame) const {
    int prevId;
    int nextId;
    if(this->anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId, frame)) {
        if(nextId == prevId) {
            return GetAsGK(this->anim_mKeys.at(nextId))->getRelFrame();
        } else {
            GraphKey *prevKey = GetAsGK(this->anim_mKeys.at(prevId));
            GraphKey *nextKey = GetAsGK(this->anim_mKeys.at(nextId));
            return getInterpolatedFrameAtRelFrame(
                        frame, prevKey, nextKey);
        }
    }
    return frame;
}

template <typename T, typename K>
void InterpolationAnimatorT<T, K>::graph_getValueConstraints(
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
