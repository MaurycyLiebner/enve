#include "animationwidgetscrollbar.h"
#include <QMouseEvent>
#include <QPainter>

int clampInt2(int val, int min, int max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

AnimationWidgetScrollBar::AnimationWidgetScrollBar(int minSpan, int maxSpan,
                                                 int spanInc, int height,
                                                 bool range,
                                                 bool clamp,
                                                 QWidget *parent) :
    QWidget(parent) {
    mMinSpan = minSpan;
    mMaxSpan = maxSpan;
    mSpanInc = spanInc;
    mRange = range;
    mClamp = clamp;
    setFramesSpan(0);

    setMinimumHeight(height);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

qreal AnimationWidgetScrollBar::posToFrame(int xPos) {
    return (xPos - 10.)*(mMaxFrame - mMinFrame + (mRange ? 0 : 1) ) /
            ((qreal)width() - 40.) + mMinFrame;
}

void AnimationWidgetScrollBar::setTopBorderVisible(const bool &bT) {
    mTopBorderVisible = bT;
}

void AnimationWidgetScrollBar::setCacheHandler(CacheHandler *handler) {
    mCacheHandler = handler;
}
#include "Boxes/rendercachehandler.h"
void AnimationWidgetScrollBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);


    p.fillRect(rect(), QColor(60, 60, 60));

    p.translate(10., 0.);
    int dFrame = mMaxFrame - mMinFrame;
    if(!mRange) dFrame++;
    qreal pixPerFrame = ((qreal)width() - 40.)/dFrame;

    QColor col = mHandleColor;
    if(mPressed) {
        col.setHsv(col.hue(),
                   col.saturation(),
                   qMin(255, col.value() + 40));
    }

    p.fillRect(QRectF((mFirstViewedFrame - mMinFrame)*pixPerFrame, 0,
               mFramesSpan*pixPerFrame, height()), col);

    p.fillRect(-10, 0, 10, height(), QColor(30, 30, 30));
    p.fillRect(width() - 40, 0, 30, height(), QColor(30, 30, 30));

    if(mCacheHandler != NULL) {
        mCacheHandler->drawCacheOnTimeline(&p, pixPerFrame, 0.,
                                           mMinFrame, mMaxFrame);
    }

    p.setPen(Qt::white);

    int divInc = 3;
    int frameInc = 5000;
    while(dFrame/frameInc < 3) {
        if(divInc == 3) {
            divInc = 0;
            frameInc *= 4;
            frameInc /= 10;
        } else {
            frameInc /= 2;
        }
        divInc++;
    }
    qreal inc = frameInc*pixPerFrame;

    int minMod = mMinFrame%frameInc;
    qreal xL = (-minMod + (mRange ? 0. : 0.5) )*pixPerFrame;
    int currentFrame = mMinFrame - minMod;
    if(!mRange) {
        while(xL < 70.) {
            currentFrame += frameInc;
            xL += inc;
        }
    }

    qreal halfHeight = height()*0.5;
    qreal qorterHeight = height()*0.25;
    qreal threeFourthsHeight = height()*0.75;
    qreal fullHeight = height();
    qreal maxX = width() + 20;
    while(xL < maxX ) {
//        p.drawLine(QPointF(xL, 0.), QPointF(xL, qorterHeight - 2 ));
        p.drawText(QRectF(xL - inc, qorterHeight, 2*inc, halfHeight),
                   Qt::AlignCenter, QString::number(currentFrame));
        p.drawLine(QPointF(xL, threeFourthsHeight + 2),
                   QPointF(xL, fullHeight ));
        xL += inc;
        currentFrame += frameInc;
    }

//    p.setPen(QPen(Qt::white, 2.));
//    p.drawLine(1, 0, 1, fullHeight);

    p.setPen(QPen(Qt::white, 1.));
    if(!mRange) {
        p.drawText(QRectF(10, 0, 100, fullHeight),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QString::number(mFirstViewedFrame));
    }

    p.setPen(QPen(Qt::black, 1.));
    p.drawLine(0, height() - 1, width(), height() - 1);
    if(mTopBorderVisible) {
        p.drawLine(0, 0, width(), 0);
    }

    p.end();
}

void AnimationWidgetScrollBar::setFramesSpan(int newSpan) {
    mFramesSpan = clampInt2(newSpan, mMinSpan, mMaxSpan);
    if(mClamp) setFirstViewedFrame(mFirstViewedFrame);
}

int AnimationWidgetScrollBar::getMaxFrame()
{
    return mMaxFrame;
}

int AnimationWidgetScrollBar::getMinFrame()
{
    return mMinFrame;
}

void AnimationWidgetScrollBar::wheelEvent(QWheelEvent *event)
{
    if(mRange) {
        int newFramesSpan = mFramesSpan;
        if(event->delta() > 0) {
            newFramesSpan -= mSpanInc;
        } else {
            newFramesSpan += mSpanInc;
        }
        setFramesSpan(newFramesSpan);
    } else {
        if(event->delta() < 0) {
            setFirstViewedFrame(mFirstViewedFrame - 1);
        } else {
            setFirstViewedFrame(mFirstViewedFrame + 1);
        }
    }

    emitChange();
    repaint();
}

bool AnimationWidgetScrollBar::setFirstViewedFrame(int firstFrame) {
    if(mClamp) {
        if(mRange) {
            mFirstViewedFrame = clampInt2(firstFrame, mMinFrame, mMaxFrame -
                                          mFramesSpan);
        } else {
            mFirstViewedFrame = clampInt2(firstFrame, mMinFrame, mMaxFrame);
        }
        return true;
    } else {
        mFirstViewedFrame = firstFrame;
        return true;
    }
    return false;
}

void AnimationWidgetScrollBar::mousePressEvent(QMouseEvent *event)
{
    mPressed = true;
    mLastMousePressFrame = posToFrame(event->x() );
    if(mLastMousePressFrame < mFirstViewedFrame ||
            mLastMousePressFrame > mFirstViewedFrame + mFramesSpan) {
        setFirstViewedFrame(qRound(mLastMousePressFrame - mFramesSpan/2.));
        emitChange();
    }
    mSavedFirstFrame = mFirstViewedFrame;
    repaint();
}

void AnimationWidgetScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    qreal newFrame = posToFrame(event->x() );
    int moveFrame = qRound(newFrame - mLastMousePressFrame);
    if(setFirstViewedFrame(clampInt2(mSavedFirstFrame + moveFrame,
                                     mMinFrame,
                                     mMaxFrame)) ) {
        emitChange();
        repaint();
    }
}

void AnimationWidgetScrollBar::mouseReleaseEvent(QMouseEvent *)
{
    mPressed = false;
    repaint();
}

void AnimationWidgetScrollBar::setViewedFramesRange(int startFrame, int endFrame)
{
    setFirstViewedFrame(startFrame);
    setFramesSpan(endFrame - startFrame);
    repaint();
}

void AnimationWidgetScrollBar::setMinMaxFrames(int minFrame, int maxFrame)
{
    mMinFrame = minFrame;
    mMaxFrame = maxFrame;
    mMaxSpan = mMaxFrame - mMinFrame;
    repaint();
}

void AnimationWidgetScrollBar::emitChange()
{
    emit viewedFramesChanged(mFirstViewedFrame,
                             mFirstViewedFrame + mFramesSpan);
}

int AnimationWidgetScrollBar::getFirstViewedFrame() {
    return mFirstViewedFrame;
}

int AnimationWidgetScrollBar::getLastViewedFrame() {
    return mFirstViewedFrame + mFramesSpan;
}
