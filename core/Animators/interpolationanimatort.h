#ifndef INTERPOLATIONANIMATORT_H
#define INTERPOLATIONANIMATORT_H
#include "graphanimatort.h"
#include "qrealpoint.h"
#include "interpolationkeyt.h"

template <typename T, typename K = InterpolationKeyT<T>>
class InterpolationAnimatorT :
        public BasedAnimatorT<GraphAnimator, K, T> {
    friend class SelfRef;
public:
    void graph_getValueConstraints(
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

protected:
    InterpolationAnimatorT(const QString& name) :
        BasedAnimatorT<GraphAnimator, K, T>(name) {}

    T getValueAtRelFrameK(const qreal frame,
                          const K * const prevKey,
                          const K * const nextKey) const {
        T result;
        const qreal prevFrame = prevKey->getRelFrame();
        const qreal nextFrame = nextKey->getRelFrame();
        const qCubicSegment1D seg{prevFrame,
                                  prevKey->getEndFrame(),
                                  nextKey->getStartFrame(),
                                  nextFrame};
        const qreal t = gTFromX(seg, frame);
        const qreal p0y = prevKey->getValueForGraph();
        const qreal p1y = prevKey->getEndValue();
        const qreal p2y = nextKey->getStartValue();
        const qreal p3y = nextKey->getValueForGraph();
        const qreal iFrame = gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
        const qreal tEff = (iFrame - prevFrame)/(nextFrame - prevFrame);
        gInterpolate(prevKey->getValue(), nextKey->getValue(),
                     tEff, result);
        return result;
    }
};

#endif // INTERPOLATIONANIMATORT_H
