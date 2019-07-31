#ifndef ANIMATIONRECT_H
#define ANIMATIONRECT_H
#include "durationrectangle.h"

class AnimationRect : public DurationRectangle {
    friend class SelfRef;
protected:
    AnimationRect(Property * const childProp) :
        DurationRectangle(childProp) {}
    virtual void setMinAnimationFrame(const int minAnimationFrame) = 0;
    virtual void setMaxAnimationFrame(const int maxAnimationFrame) = 0;
public:
    virtual int getMinAnimationFrame() const = 0;
    virtual int getMaxAnimationFrame() const = 0;

    bool hasAnimationFrameRange() { return true; }

    void draw(QPainter * const p,
              const QRect &drawRect,
              const qreal fps,
              const qreal pixelsPerFrame,
              const FrameRange &absFrameRange);

    FrameRange getAnimationRange() const {
        return { getMinAnimationFrameAsRelFrame(),
                 getMaxAnimationFrameAsRelFrame()};
    }

    int getMaxAnimationFrameAsRelFrame() const;
    int getMinAnimationFrameAsRelFrame() const;

    int getMaxAnimationFrameAsAbsFrame() const;
    int getMinAnimationFrameAsAbsFrame() const;

    void setAnimationFrameDuration(const int frameDuration);

    int getAnimationFrameDuration();
};

#endif // ANIMATIONRECT_H
