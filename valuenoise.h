#ifndef VALUENOISE_H
#define VALUENOISE_H
#include "qrealanimator.h"

struct NoisePoint {
    NoisePoint(qreal frameT, qreal valueT) {
        frame = frameT;
        value = valueT;
    }

    qreal value;
    qreal frame;
};

class ValueNoise
{
public:
    ValueNoise();
    void generateNoisePoints();
    qreal getValueAtFrame(int frame) const;
    qreal getCurrentValue() const;
    void setFrame(int frame);
    void updateCurrentValue();
private:
    qreal getValueAtFrame(int frame,
                          NoisePoint prevNoisePoint, NoisePoint nextNoisePoint) const;
    bool getNextAndPreviousNoisePointId(int *prevIdP, int *nextIdP, int frame) const;
    QrealAnimator mAmplitudeAnimator;
    QrealAnimator mFramePeriodAnimator;
    QList<NoisePoint> mNoisePoints;

    int mFrame = 0;
    qreal mCurrentValue = 0.;
};

#endif // VALUENOISE_H
