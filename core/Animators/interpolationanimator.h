#ifndef INTERPOLATIONANIMATOR_H
#define INTERPOLATIONANIMATOR_H
#include "graphanimator.h"

class InterpolationAnimator : public GraphAnimator {
public:
    InterpolationAnimator(const QString& name);
    qreal getInterpolatedFrameAtRelFrameF(const qreal &frame) const;

    void getValueConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minValue, qreal &maxValue) const;
private:
    qreal getInterpolatedFrameAtRelFrameF(const qreal &frame,
                                          GraphKey *prevKey,
                                          GraphKey *nextKey) const;
};

#endif // INTERPOLATIONANIMATOR_H
