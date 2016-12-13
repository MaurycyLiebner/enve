#include "animatonwidgetscrollbar.h"
#include <QMouseEvent>
#include <QPainter>

int clampInt2(int val, int min, int max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

AnimatonWidgetScrollBar::AnimatonWidgetScrollBar(int minSpan, int maxSpan,
                                                 int spanInc, int height,
                                                 bool range,
                                                 bool clamp,
                                                 QWidget *parent) :
    QWidget(parent)
{
    mMinSpan = minSpan;
    mMaxSpan = maxSpan;
    mSpanInc = spanInc;
    mRange = range;
    mClamp = clamp;
    setFramesSpan(0);

    setMinimumHeight(height);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

qreal AnimatonWidgetScrollBar::posToFrame(int xPos)
{
    return xPos*(mMaxFrame - mMinFrame + (mRange ? 0 : 1) ) /
            ((qreal)width()) + mMinFrame;
}

void AnimatonWidgetScrollBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setPen(Qt::NoPen);

    p.fillRect(rect(), QColor(75, 75, 75));

    QColor col;
    if(mPressed) {
        col = QColor(255, 0, 0);
    } else {
        col = QColor(0, 0, 0);
    }
    p.setBrush(col);
    int dFrame = mMaxFrame - mMinFrame;
    if(!mRange) dFrame++;
    qreal pixPerFrame = ((qreal)width())/dFrame;
    p.drawRect(QRectF((mFirstViewedFrame - mMinFrame)*pixPerFrame, 0,
               mFramesSpan*pixPerFrame, height()) );

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
        p.drawLine(QPointF(xL, 0.), QPointF(xL, qorterHeight - 2 ));
        p.drawText(QRectF(xL - inc, qorterHeight, 2*inc, halfHeight),
                   Qt::AlignCenter, QString::number(currentFrame));
        p.drawLine(QPointF(xL, threeFourthsHeight + 2),
                   QPointF(xL, fullHeight ));
        xL += inc;
        currentFrame += frameInc;
    }

    p.setPen(QPen(Qt::white, 2.));
    p.drawLine(1, 0, 1, fullHeight);

    if(!mRange) {
        p.drawText(QRectF(10, 0, 100, fullHeight),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   QString::number(mFirstViewedFrame));
    }

    p.end();
}

void AnimatonWidgetScrollBar::setFramesSpan(int newSpan) {
    mFramesSpan = clampInt2(newSpan, mMinSpan, mMaxSpan);
    if(mClamp) setFirstViewedFrame(mFirstViewedFrame);
}

int AnimatonWidgetScrollBar::getMaxFrame()
{
    return mMaxFrame;
}

int AnimatonWidgetScrollBar::getMinFrame()
{
    return mMinFrame;
}

void AnimatonWidgetScrollBar::wheelEvent(QWheelEvent *event)
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

bool AnimatonWidgetScrollBar::setFirstViewedFrame(int firstFrame) {
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

void AnimatonWidgetScrollBar::mousePressEvent(QMouseEvent *event)
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

void AnimatonWidgetScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    qreal newFrame = posToFrame(event->x() );
    int moveFrame = qRound(newFrame - mLastMousePressFrame);
    if(setFirstViewedFrame(mSavedFirstFrame + moveFrame) ) {
        emitChange();
        repaint();
    }
}

void AnimatonWidgetScrollBar::mouseReleaseEvent(QMouseEvent *)
{
    mPressed = false;
    repaint();
}

void AnimatonWidgetScrollBar::setViewedFramesRange(int startFrame, int endFrame)
{
    setFirstViewedFrame(startFrame);
    setFramesSpan(endFrame - startFrame);
    repaint();
}

void AnimatonWidgetScrollBar::setMinMaxFrames(int minFrame, int maxFrame)
{
    mMinFrame = minFrame;
    mMaxFrame = maxFrame;
    repaint();
}

void AnimatonWidgetScrollBar::emitChange()
{
    emit viewedFramesChanged(mFirstViewedFrame,
                             mFirstViewedFrame + mFramesSpan);
}
