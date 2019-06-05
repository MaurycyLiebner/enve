#ifndef INTERPOLATIONANIMATOR_H
#define INTERPOLATIONANIMATOR_H
#include "graphanimator.h"

class InterpolationAnimator : public GraphAnimator {
public:
    InterpolationAnimator(const QString& name);

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minValue, qreal &maxValue) const;
    qreal getInterpolatedFrameAtRelFrame(const qreal frame) const;
};

#endif // INTERPOLATIONANIMATOR_H
