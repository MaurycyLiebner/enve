#ifndef INTERPOLATIONANIMATOR_H
#define INTERPOLATIONANIMATOR_H
#include "graphanimator.h"

class InterpolationAnimator : public GraphAnimator {
public:
    InterpolationAnimator(const QString& name);
    qreal getInterpolatedFrameAtRelFrame(const qreal &frame) const;

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minValue, qreal &maxValue) const;
private:
    qreal getInterpolatedFrameAtRelFrame(const qreal &frame,
                                         GraphKey *prevKey,
                                         GraphKey *nextKey) const;
};

#endif // INTERPOLATIONANIMATOR_H
