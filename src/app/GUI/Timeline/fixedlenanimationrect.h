#ifndef FIXEDLENANIMATIONRECT_H
#define FIXEDLENANIMATIONRECT_H
#include "animationrect.h"

class FixedLenAnimationRect : public AnimationRect {
public:
    FixedLenAnimationRect(Property *childProp) : AnimationRect(childProp) {
        mType = FIXED_LEN_ANIMATION_LEN;
    }

    int getMinAnimationFrame() const;
    int getMaxAnimationFrame() const;

    void changeFramePosBy(const int change);
    void writeDurationRectangle(QIODevice *target);
    void readDurationRectangle(QIODevice *target);

    void openDurationSettingsDialog(QWidget *parent = nullptr);

    void setFirstAnimationFrame(const int minAnimationFrame) {
        const int animDur = mMaxAnimationFrame - mMinAnimationFrame;
        setMinAnimationFrame(minAnimationFrame);
        setMaxAnimationFrame(minAnimationFrame + animDur);
    }

    void bindToAnimationFrameRange();
    void setBindToAnimationFrameRange();
protected:
    void setMinAnimationFrame(const int minAnimationFrame);
    void setMaxAnimationFrame(const int maxAnimationFrame);

    bool mBoundToAnimation = false;
    bool mSetMaxFrameAtLeastOnce = false;
    int mMinAnimationFrame = 0;
    int mMaxAnimationFrame = 100;
};

#endif // FIXEDLENANIMATIONRECT_H
