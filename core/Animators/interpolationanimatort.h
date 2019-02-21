#ifndef INTERPOLATIONANIMATORT_H
#define INTERPOLATIONANIMATORT_H
#include "graphkeyt.h"
#include "graphanimatort.h"
#include "differsinterpolate.h"
#include "qrealpoint.h"

template <typename T>
class InterpolationKeyT : public GraphKeyT<T> {
    friend class StdSelfRef;
public:
    qreal getValueForGraph() const {
        return this->mRelFrame;
    }

    void setValueForGraph(const qreal& value) {
        Q_UNUSED(value);
    }

    void setRelFrame(const int &frame) {
        if(frame == this->mRelFrame) return;
        const int dFrame = frame - this->mRelFrame;
        GraphKeyT<T>::setRelFrame(frame);
        this->mEndValue += dFrame;
        this->mStartValue += dFrame;
    }
protected:
    InterpolationKeyT(const T &value, const int &frame,
                      Animator * const parentAnimator) :
        GraphKeyT<T>(value, frame, parentAnimator) {}
    InterpolationKeyT(Animator * const parentAnimator) :
        GraphKeyT<T>(parentAnimator) {}
};

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

    T getValueAtRelFrameK(const qreal &frame,
                          const K * const prevKey,
                          const K * const nextKey) const {
        T result;
        const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                                  prevKey->getEndFrame(),
                                  nextKey->getStartFrame(),
                                  qreal(nextKey->getRelFrame())};
        const qreal t = gTFromX(seg, frame);
        const qreal p0y = prevKey->getValueForGraph();
        const qreal p1y = prevKey->getEndValue();
        const qreal p2y = nextKey->getStartValue();
        const qreal p3y = nextKey->getValueForGraph();
        const qreal iFrame = gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
        gInterpolate(prevKey->getValue(), nextKey->getValue(),
                     iFrame, result);
        return result;
    }
};

#endif // INTERPOLATIONANIMATORT_H
