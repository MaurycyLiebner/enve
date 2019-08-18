#ifndef ANIMATIONRECT_H
#define ANIMATIONRECT_H
#include "durationrectangle.h"

class AnimationRect : public DurationRectangle {
    e_OBJECT
protected:
    AnimationRect(Property& parentProp) :
        DurationRectangle(parentProp) {}
    virtual void setMinAnimRelFrame(const int min) = 0;
    virtual void setMaxAnimRelFrame(const int max) = 0;
public:
    virtual int getMinAnimRelFrame() const = 0;
    virtual int getMaxAnimRelFrame() const = 0;

    bool hasAnimationFrameRange() { return true; }

    void draw(QPainter * const p,
              const QRect &drawRect,
              const qreal fps,
              const qreal pixelsPerFrame,
              const FrameRange &absFrameRange);

    FrameRange getAnimRelRange() const {
        return { getMinAnimRelFrame(), getMaxAnimRelFrame()};
    }

    int getMinAnimAbsFrame() const;
    int getMaxAnimAbsFrame() const;

    void setAnimationFrameDuration(const int frameDuration);

    int getAnimationFrameDuration();
};

#endif // ANIMATIONRECT_H
