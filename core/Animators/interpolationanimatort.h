#ifndef INTERPOLATIONANIMATORT_H
#define INTERPOLATIONANIMATORT_H
#include "graphkeyt.h"
#include "graphanimatort.h"
#include "differsinterpolate.h"

template <typename T>
class InterpolationKeyT : public GraphKeyT<T> {
public:
    InterpolationKeyT(const T &value, const int &frame,
                      Animator * const parentAnimator) :
        GraphKeyT<T>(value, frame, parentAnimator) {}
    InterpolationKeyT(Animator * const parentAnimator) :
        GraphKeyT<T>(parentAnimator) {}

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
};

template <typename T, typename K = InterpolationKeyT<T>>
class InterpolationAnimatorT :
        public BasedAnimatorT<GraphAnimator, K, T> {
public:

    void graph_getValueConstraints(GraphKey *key,
                                   const QrealPointType &type,
                                   qreal &minValue, qreal &maxValue) const;
protected:
    InterpolationAnimatorT(const QString& name) :
        BasedAnimatorT<GraphAnimator, K, T>(name) {}

    T getValueAtRelFrameK(const qreal &frame,
                          const K * const prevKey,
                          const K * const nextKey) const {
        T result;
        gInterpolate(prevKey->getValue(), nextKey->getValue(),
                     getInterpolatedFrameAtRelFrame(frame), result);
        return result;
    }
    qreal getInterpolatedFrameAtRelFrame(const qreal &frame) const;
};

#endif // INTERPOLATIONANIMATORT_H
