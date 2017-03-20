#include "durationrectangle.h"

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

DurationRectangle::DurationRectangle() : DurationRectangleMovable() {
    setMinPos(-1000000);
    setMaxPos(1000000);
    mMinFrame.setMinPos(-1000000);
    mMaxFrame.setMaxPos(1000000);
    connect(&mMinFrame, SIGNAL(posChanged(int)),
            &mMaxFrame, SLOT(setMinPos(int)));
    connect(&mMaxFrame, SIGNAL(posChanged(int)),
            &mMinFrame, SLOT(setMaxPos(int)));

    connect(&mMinFrame, SIGNAL(posChanged(int)),
            this, SIGNAL(changed()));
    connect(&mMaxFrame, SIGNAL(posChanged(int)),
            this, SIGNAL(changed()));
    connect(this, SIGNAL(posChanged(int)),
            this, SIGNAL(changed()));
}

void DurationRectangle::setFramesDuration(const int &duration) {
    mMaxFrame.setFramePos(getMinFrame() + duration);
}

void DurationRectangle::setMinFrame(const int &minFrame) {
    mMinFrame.setFramePos(minFrame);
}

void DurationRectangle::setMaxFrame(const int &maxFrame) {
    mMaxFrame.setFramePos(maxFrame);
}

int DurationRectangle::getFrameDuration() const {
    return mMaxFrame.getFramePos() - mMinFrame.getFramePos();
}

int DurationRectangle::getMinFrame() const {
    return mMinFrame.getFramePos();
}

int DurationRectangle::getMaxFrame() const {
    return mMaxFrame.getFramePos();
}

void DurationRectangle::draw(QPainter *p, const qreal &pixelsPerFrame,
                             const qreal &drawY, const int &startFrame) {
    int startDFrame;
    int xT;
    int widthT;
    if(mShowPossibleFrameRange) {
        startDFrame = getMinPossibleFrame() - startFrame;
        xT = startDFrame*pixelsPerFrame + pixelsPerFrame*0.5;
        widthT = getPossibleFrameDuration()*pixelsPerFrame - pixelsPerFrame;
        p->fillRect(xT, drawY,
                    widthT,
                    BOX_HEIGHT, QColor(125, 125, 255, 180));
    }

    startDFrame = getMinFrame() - startFrame;
    xT = startDFrame*pixelsPerFrame + pixelsPerFrame*0.5;
    widthT = getFrameDuration()*pixelsPerFrame - pixelsPerFrame;
    QColor fillColor;
    if(mHovered) {
        fillColor = QColor(50, 50, 255, 180);
    } else {
        fillColor = QColor(0, 0, 255, 180);
    }
    p->fillRect(xT, drawY,
                widthT,
                BOX_HEIGHT, fillColor);

    if(mMinFrame.isHovered()) {
        p->setPen(QPen(Qt::white));
    } else {
        p->setPen(QPen(Qt::black));
    }
    p->drawLine(QPoint(xT, drawY), QPoint(xT, drawY + BOX_HEIGHT));
    xT += widthT;
    if(mMaxFrame.isHovered()) {
        p->setPen(QPen(Qt::white));
    } else {
        p->setPen(QPen(Qt::black));
    }
    p->drawLine(QPoint(xT, drawY), QPoint(xT, drawY + BOX_HEIGHT));
    p->setPen(Qt::black);
}

DurationRectangleMovable *DurationRectangle::getMovableAt(
                                          const int &pressX,
                                          const qreal &pixelsPerFrame,
                                          const int &minViewedFrame) {
    qreal startX = (getMinFrame() - minViewedFrame + 0.5)*pixelsPerFrame;
    qreal endX = (getMaxFrame() - minViewedFrame - 0.5)*pixelsPerFrame;
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
    mMaxPossibleFrame += change;
    mMinPossibleFrame += change;
    mMinFrame.changeFramePosByWithoutSignal(change);
    mMaxFrame.setMinPos(getMinFrame());
    mMaxFrame.changeFramePosByWithoutSignal(change);
    mMinFrame.setMaxPos(getMaxFrame());
    DurationRectangleMovable::changeFramePosBy(change);
}

void DurationRectangle::setPossibleFrameRangeVisible() {
    mShowPossibleFrameRange = true;
}

int DurationRectangle::getMinPossibleFrame() {
    return mMinPossibleFrame;
}

int DurationRectangle::getMaxPossibleFrame() {
    return mMaxPossibleFrame;
}

void DurationRectangle::setMinPossibleFrame(const int &minPossibleFrame) {
    mMinPossibleFrame = minPossibleFrame;
}

void DurationRectangle::setMaxPossibleFrame(const int &maxPossibleFrame) {
    mMaxPossibleFrame = maxPossibleFrame;
    if(mSetMaxFrameAtLeastOnce) return;
    mSetMaxFrameAtLeastOnce = true;
    setMaxFrame(maxPossibleFrame);
    mMinFrame.setMaxPos(maxPossibleFrame);
}

void DurationRectangle::setPossibleFrameDuration(const int &frameDuration) {
    setMaxPossibleFrame(mMinPossibleFrame + frameDuration);
}

int DurationRectangle::getPossibleFrameDuration() {
    return mMaxPossibleFrame - mMinPossibleFrame;
}
