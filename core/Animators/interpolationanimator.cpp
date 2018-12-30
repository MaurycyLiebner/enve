#include "interpolationanimator.h"
#include "graphkey.h"

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

qreal InterpolationAnimator::getInterpolatedFrameAtRelFrameF(
        const qreal &frame,
        GraphKey *prevKey, GraphKey *nextKey) const {
    qreal t = tFromX(prevKey->getRelFrame(),
                     prevKey->getEndFrame(),
                     nextKey->getStartFrame(),
                     nextKey->getRelFrame(), frame);
    qreal p0y = prevKey->getValueForGraph();
    qreal p1y = prevKey->getEndValue();
    qreal p2y = nextKey->getStartValue();
    qreal p3y = nextKey->getValueForGraph();
    return calcCubicBezierVal(p0y, p1y, p2y, p3y, t);
}
