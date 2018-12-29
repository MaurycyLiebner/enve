#ifndef INTERPOLATIONANIMATOR_H
#define INTERPOLATIONANIMATOR_H
#include "graphanimator.h"

class InterpolationAnimator : public GraphAnimator {
public:
    InterpolationAnimator(const QString& name);
    void getValueConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minMoveValue, qreal &maxMoveValue) const {
        getFrameConstraints(key, type, minMoveValue, maxMoveValue);
    }
    qreal getInterpolatedFrameAtRelFrameF(const qreal &frame) const;
private:
    qreal getInterpolatedFrameAtRelFrameF(const qreal &frame,
                                          GraphKey *prevKey,
                                          GraphKey *nextKey) const;
};

#endif // INTERPOLATIONANIMATOR_H
