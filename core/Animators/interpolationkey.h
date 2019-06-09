#ifndef INTERPOLATIONKEY_H
#define INTERPOLATIONKEY_H
#include "graphkey.h"

class InterpolationKey : public GraphKey {
public:
    InterpolationKey(const int frame,
                     Animator * const parentAnimator);
    InterpolationKey(Animator * const parentAnimator);

    qreal getValueForGraph() const {
        return mRelFrame;
    }

    void setValueForGraph(const qreal value) {
        Q_UNUSED(value);
    }

    void setRelFrame(const int frame) {
        if(frame == this->mRelFrame) return;
        const int dFrame = frame - mRelFrame;
        GraphKey::setRelFrame(frame);
        setStartValueVar(mStartPt.getRawYValue() + dFrame);
        setEndValueVar(mEndPt.getRawYValue() + dFrame);
    }
};

#endif // INTERPOLATIONKEY_H
