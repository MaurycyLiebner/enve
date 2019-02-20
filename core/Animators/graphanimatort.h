#ifndef GRAPHANIMATORT_H
#define GRAPHANIMATORT_H
#include "graphkeyt.h"
#include "basedanimatort.h"
#include "graphanimator.h"

template <typename T>
class GraphAnimatorT : public BasedAnimatorT<GraphAnimator, GraphKeyT<T>, T> {
public:
    T getValueAtRelFrame(const qreal &frame,
                         const GraphKeyT<T> * const prevKey,
                         const GraphKeyT<T> * const nextKey) const {
        const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                                  prevKey->getEndFrame(),
                                  nextKey->getStartFrame(),
                                  qreal(nextKey->getRelFrame())};
        const qreal t = gTFromX(seg, frame);
        return gInterpolate(prevKey->getValue(), nextKey->getValue(), t);
    }
protected:
    GraphAnimatorT(const QString& name) :
        BasedAnimatorT<GraphAnimator, GraphKeyT<T>, T>(name) {}
};

#endif // GRAPHANIMATORT_H
