#include "durationrectangle.h"
#include "Properties/property.h"
#include "Boxes/rendercachehandler.h"
#include "global.h"

DurationRectangleMovable::DurationRectangleMovable() : QObject() {

}

void DurationRectangleMovable::setFramePos(const int &framePos) {
    mFramePos = qMin(mMaxPos, qMax(mMinPos, framePos));
}

int DurationRectangleMovable::getFramePos() const {
    return mFramePos;
}

DurationRectangleMovable *DurationRectangleMovable::getMovableAt(
                                const int &pressX,
                                const qreal &pixelsPerFrame,
                                const int &minViewedFrame) {
    int pressedDFrame = qRound(minViewedFrame + pressX/pixelsPerFrame);
    int thisDFrame = mFramePos - minViewedFrame;
    if(thisDFrame == pressedDFrame) {
        return this;
    }
    return NULL;
}

void DurationRectangleMovable::changeFramePosBy(const int &change) {
    changeFramePosByWithoutSignal(change);
    emit posChanged(mFramePos);
    emit posChangedBy(change);
}

void DurationRectangleMovable::changeFramePosByWithoutSignal(
                                const int &change) {
    setFramePos(mFramePos + change);
}

void DurationRectangleMovable::setHovered(const bool &hovered) {
    mHovered = hovered;
}

bool DurationRectangleMovable::isHovered() {
    return mHovered;
}

void DurationRectangleMovable::setMaxPos(const int &maxPos) {
    mMaxPos = maxPos - 1;
}

void DurationRectangleMovable::setMinPos(const int &minPos) {
    mMinPos = minPos + 1;
}

DurationRectangle::DurationRectangle(Property *childProp) :
    DurationRectangleMovable() {
    mChildProperty = childProp;

    setMinPos(-1000000);
    setMaxPos(1000000);
    mMinFrame.setMaxPos(1000000);
    mMinFrame.setMinPos(-1000000);
    mMaxFrame.setMinPos(-1000000);
    mMaxFrame.setMaxPos(1000000);
    connect(&mMinFrame, SIGNAL(posChanged(int)),
            &mMaxFrame, SLOT(setMinPos(int)));
    connect(&mMaxFrame, SIGNAL(posChanged(int)),
            &mMinFrame, SLOT(setMaxPos(int)));

    connect(&mMinFrame, SIGNAL(posChanged(int)),
            this, SIGNAL(rangeChanged()));
    connect(&mMaxFrame, SIGNAL(posChanged(int)),
            this, SIGNAL(rangeChanged()));

    connect(&mMinFrame, SIGNAL(finishedTransform()),
            this, SIGNAL(finishedRangeChange()));
    connect(&mMaxFrame, SIGNAL(finishedTransform()),
            this, SIGNAL(finishedRangeChange()));

    connect(&mMinFrame, SIGNAL(posChangedBy(int)),
            this, SIGNAL(minFrameChangedBy(int)));
    connect(&mMaxFrame, SIGNAL(posChangedBy(int)),
            this, SIGNAL(maxFrameChangedBy(int)));
}

void DurationRectangle::setFramesDuration(const int &duration) {
    mMaxFrame.setFramePos(getMinFrame() + duration - 1);
}

void DurationRectangle::setMinFrame(const int &minFrame) {
    mMinFrame.setFramePos(minFrame);
}

void DurationRectangle::setMaxFrame(const int &maxFrame) {
    mMaxFrame.setFramePos(maxFrame);
}

int DurationRectangle::getFrameDuration() const {
    return mMaxFrame.getFramePos() - mMinFrame.getFramePos() + 1;
}

int DurationRectangle::getMinFrame() const {
    return mMinFrame.getFramePos();
}

int DurationRectangle::getMaxFrame() const {
    return mMaxFrame.getFramePos();
}

int DurationRectangle::getMinFrameAsRelFrame() const {
    return getMinFrame() - mFramePos;
}

int DurationRectangle::getMaxFrameAsRelFrame() const {
    return getMaxFrame() - mFramePos;
}

int DurationRectangle::getMinFrameAsAbsFrame() const {
    return mChildProperty->prp_relFrameToAbsFrame(
                getMinFrameAsRelFrame());
}

int DurationRectangle::getMaxFrameAsAbsFrame() const {
    return mChildProperty->prp_relFrameToAbsFrame(
                getMaxFrameAsRelFrame());
}

void DurationRectangle::draw(QPainter *p, const qreal &pixelsPerFrame,
                             const qreal &drawY, const int &startFrame) {
    p->save();
    int startDFrame;
    int xT;
    int widthT;
    QRect drawRect;

    startDFrame = getMinFrame() - startFrame;
    xT = startDFrame*pixelsPerFrame + pixelsPerFrame*0.5;
    widthT = getFrameDuration()*pixelsPerFrame - pixelsPerFrame;
    QColor fillColor;
    if(mHovered) {
        fillColor = QColor(50, 50, 255, 120);
    } else {
        fillColor = QColor(0, 0, 255, 120);
    }
    drawRect = QRect(xT, drawY,
                     widthT, MIN_WIDGET_HEIGHT);

    p->fillRect(drawRect.adjusted(0, 1, 0, -1), fillColor);

    if(mMinFrame.isHovered()) {
        p->setPen(QPen(Qt::white));
    } else {
        p->setPen(QPen(Qt::black));
    }
    p->drawLine(QPoint(xT, drawY), QPoint(xT, drawY + MIN_WIDGET_HEIGHT));
    xT += widthT;
    if(mMaxFrame.isHovered()) {
        p->setPen(QPen(Qt::white));
    } else {
        p->setPen(QPen(Qt::black));
    }
    p->drawLine(QPoint(xT, drawY), QPoint(xT, drawY + MIN_WIDGET_HEIGHT));
//    p->setPen(Qt::black);
//    p->setBrush(Qt::NoBrush);
    //p->drawRect(drawRect);
    p->restore();
}

DurationRectangleMovable *DurationRectangle::getMovableAt(
                                          const int &pressX,
                                          const qreal &pixelsPerFrame,
                                          const int &minViewedFrame) {
    qreal startX = (getMinFrame() - minViewedFrame + 0.5)*pixelsPerFrame;
    qreal endX = (getMaxFrame() - minViewedFrame + 0.5)*pixelsPerFrame;
    if(qAbs(pressX - startX) < 5.) {
        return &mMinFrame;
    } else if(qAbs(pressX - endX) < 5.) {
        return &mMaxFrame;
    } else if(pressX > startX && pressX < endX) {
        return this;
    }
    return NULL;
}

void DurationRectangle::changeFramePosBy(const int &change) {
    mMinFrame.changeFramePosByWithoutSignal(change);
    mMaxFrame.setMinPos(getMinFrame());
    mMaxFrame.changeFramePosByWithoutSignal(change);
    mMinFrame.setMaxPos(getMaxFrame());
    DurationRectangleMovable::changeFramePosBy(change);
}

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

void AnimationRect::setAnimationFrameDuration(const int &frameDuration) {
    setMaxAnimationFrame(getMinAnimationFrame() + frameDuration);
}

int AnimationRect::getAnimationFrameDuration() {
    return getMaxAnimationFrame() - getMinAnimationFrame();
}

void AnimationRect::draw(
        QPainter *p, const qreal &pixelsPerFrame,
        const qreal &drawY, const int &startFrame) {
    p->save();

    int startDFrame = getMinAnimationFrame() - startFrame;
    int xT = startDFrame*pixelsPerFrame + pixelsPerFrame*0.5;
    int widthT = getAnimationFrameDuration()*pixelsPerFrame - pixelsPerFrame;
    QRect drawRect = QRect(xT, drawY,
                           widthT,
                           MIN_WIDGET_HEIGHT);
    p->fillRect(drawRect.adjusted(0, 1, 0, -1), QColor(125, 125, 255, 180));

    DurationRectangle::draw(p, pixelsPerFrame,
                            drawY, startFrame);
    p->restore();
}

int FixedLenAnimationRect::getMinAnimationFrame() const {
    return mMinAnimationFrame;
}

int FixedLenAnimationRect::getMaxAnimationFrame() const {
    return mMaxAnimationFrame;
}

void FixedLenAnimationRect::bindToAnimationFrameRange() {
    if(getMinFrame() < getMinAnimationFrame() ||
       getMinFrame() >= getMaxAnimationFrame()) {
        setMinFrame(getMinAnimationFrame());
    }
    mMinFrame.setMinPos(getMinAnimationFrame());
    if(getMaxFrame() > getMaxAnimationFrame() ||
       getMaxFrame() <= getMinAnimationFrame()) {
        setMaxFrame(getMaxAnimationFrame());
    }
    mMaxFrame.setMaxPos(getMaxAnimationFrame());
    emit rangeChanged();
}

void FixedLenAnimationRect::setBindToAnimationFrameRange() {
    mBoundToAnimation = true;
}

void FixedLenAnimationRect::setMinAnimationFrame(const int &minAnimationFrame) {
    mMinAnimationFrame = minAnimationFrame;
    if(mBoundToAnimation) {
        bindToAnimationFrameRange();
    }
}

void FixedLenAnimationRect::setMaxAnimationFrame(const int &maxAnimationFrame) {
    bool moveMaxPosFrame = getMinFrame() == mFramePos &&
                           getMaxFrame() == mFramePos + getAnimationFrameDuration();
    mMaxAnimationFrame = maxAnimationFrame;
    if(mSetMaxFrameAtLeastOnce) {
        if(moveMaxPosFrame) {
            setMaxFrame(mMaxAnimationFrame);
        }
    } else {
        mSetMaxFrameAtLeastOnce = true;
        setMaxFrame(maxAnimationFrame);
        mMinFrame.setMaxPos(maxAnimationFrame);
    }
    if(mBoundToAnimation) {
        bindToAnimationFrameRange();
    }
}

void FixedLenAnimationRect::changeFramePosBy(const int &change) {
    mMaxAnimationFrame += change;
    mMinAnimationFrame += change;
    mMinFrame.changeFramePosByWithoutSignal(change);
    mMaxFrame.setMinPos(getMinFrame());
    mMaxFrame.changeFramePosByWithoutSignal(change);
    mMinFrame.setMaxPos(getMaxFrame());
    if(mBoundToAnimation) {
        bindToAnimationFrameRange();
    }
    DurationRectangleMovable::changeFramePosBy(change);
}
