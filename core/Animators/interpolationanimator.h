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
};

#endif // INTERPOLATIONANIMATOR_H
