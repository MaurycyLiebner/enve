#include "animationrect.h"
#include "Properties/property.h"

int AnimationRect::getMaxAnimationFrameAsRelFrame() const {
    return getMaxAnimationFrame() - mFramePos;
}

int AnimationRect::getMinAnimationFrameAsRelFrame() const {
    return getMinAnimationFrame() - mFramePos;
}

int AnimationRect::getMaxAnimationFrameAsAbsFrame() const {
    return mChildProperty->prp_relFrameToAbsFrame(
                getMaxAnimationFrameAsRelFrame());
}

int AnimationRect::getMinAnimationFrameAsAbsFrame() const {
    return mChildProperty->prp_relFrameToAbsFrame(
                getMinAnimationFrameAsRelFrame());
}

void AnimationRect::setAnimationFrameDuration(const int frameDuration) {
    const int oldMinFrame = getMinFrame();
    const int oldMaxFrame = getMaxFrame();
    const int oldMinAnimFrame = getMinAnimationFrame();
    const int oldMaxAnimFrame = getMaxAnimationFrame();

    setMaxAnimationFrame(getMinAnimationFrame() + frameDuration - 1);

    const int newMinFrame = getMinFrame();
    const int newMaxFrame = getMaxFrame();
    const int newMinAnimFrame = getMinAnimationFrame();
    const int newMaxAnimFrame = getMaxAnimationFrame();

    if(oldMinFrame != newMinFrame) {
        const int minMinFrame = qMin(oldMinFrame, newMinFrame);
        const int maxMinFrame = qMax(oldMinFrame, newMinFrame);
        mChildProperty->prp_afterChangedRelRange(
                                {minMinFrame + 1, maxMinFrame});
    }
    if(oldMaxFrame != newMaxFrame) {
        const int minMaxFrame = qMin(oldMaxFrame, newMaxFrame);
        const int maxMaxFrame = qMax(oldMaxFrame, newMaxFrame);
        mChildProperty->prp_afterChangedRelRange(
                                {minMaxFrame, maxMaxFrame - 1});
    }

    if(oldMinAnimFrame != newMinAnimFrame) {
        const int minMinAnimFrame = qMin(oldMinAnimFrame, newMinAnimFrame);
        const int maxMinAnimFrame = qMax(oldMinAnimFrame, newMinAnimFrame);
        mChildProperty->prp_afterChangedRelRange(
                    {minMinAnimFrame, maxMinAnimFrame - 1});
    }

    if(oldMaxAnimFrame != newMaxAnimFrame) {
        const int minMaxAnimFrame = qMin(oldMaxAnimFrame, newMaxAnimFrame);
        const int maxMaxAnimFrame = qMax(oldMaxAnimFrame, newMaxAnimFrame);
        mChildProperty->prp_afterChangedRelRange(
                    {minMaxAnimFrame + 1, maxMaxAnimFrame});
    }
}

int AnimationRect::getAnimationFrameDuration() {
    return getMaxAnimationFrame() - getMinAnimationFrame() + 1;
}

void AnimationRect::draw(QPainter * const p,
                         const QRect& drawRect,
                         const qreal fps,
                         const qreal pixelsPerFrame,
                         const FrameRange &absFrameRange) {
    const int firstRelDrawFrame =
            qMax(absFrameRange.fMin, getMinAnimationFrame()) - absFrameRange.fMin;
    const int lastRelDrawFrame =
            qMin(absFrameRange.fMax, getMaxAnimationFrame()) - absFrameRange.fMin;
    const int drawFrameSpan = lastRelDrawFrame - firstRelDrawFrame + 1;
    if(drawFrameSpan < 1)
        return DurationRectangle::draw(p, drawRect, fps,
                                       pixelsPerFrame, absFrameRange);

    QRect animDurRect(qFloor(firstRelDrawFrame*pixelsPerFrame), drawRect.y(),
                      qCeil(drawFrameSpan*pixelsPerFrame), drawRect.height());

    p->fillRect(animDurRect.adjusted(0, 1, 0, -1), QColor(125, 125, 255, 180));

    DurationRectangle::draw(p, drawRect, fps, pixelsPerFrame, absFrameRange);
}
