#ifndef FIXEDLENANIMATIONRECT_H
#define FIXEDLENANIMATIONRECT_H
#include "animationrect.h"

class FixedLenAnimationRect : public AnimationRect {
public:
    FixedLenAnimationRect(Property& parentProp) :
        AnimationRect(parentProp) {}

    int getMinAnimRelFrame() const;
    int getMaxAnimRelFrame() const;

    void writeDurationRectangle(QIODevice *target);
    void readDurationRectangle(QIODevice *target);

    void setFirstAnimationFrame(const int minAnimationFrame) {
        const int animDur = mMaxAnimationFrame - mMinAnimationFrame;
        setMinAnimRelFrame(minAnimationFrame);
        setMaxAnimRelFrame(minAnimationFrame + animDur);
    }

    void bindToAnimationFrameRange();
    void setBindToAnimationFrameRange();
protected:
    void setMinAnimRelFrame(const int min);
    void setMaxAnimRelFrame(const int max);

    bool mBoundToAnimation = false;
    bool mSetMaxFrameAtLeastOnce = false;
    int mMinAnimationFrame = 0;
    int mMaxAnimationFrame = 0;
};

#endif // FIXEDLENANIMATIONRECT_H
