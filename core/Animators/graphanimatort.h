#ifndef GRAPHANIMATORT_H
#define GRAPHANIMATORT_H
#include "graphkeyt.h"
#include "basedanimatort.h"
#include "graphanimator.h"

template <typename T>
class GraphAnimatorT : public BasedAnimatorT<GraphAnimator, GraphKeyT<T>, T> {
public:

protected:
    GraphAnimatorT(const QString& name) :
        BasedAnimatorT<GraphAnimator, GraphKeyT<T>, T>(name) {}
};

#endif // GRAPHANIMATORT_H
