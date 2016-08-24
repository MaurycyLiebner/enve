#include "animatonwidgetscrollbar.h"
#include <QMouseEvent>
#include <QPainter>

AnimatonWidgetScrollBar::AnimatonWidgetScrollBar(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(30, 30);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

int AnimatonWidgetScrollBar::posToFrame(int xPos)
{
    return xPos*(mMaxFrame - mMinFrame)/width() + mMinFrame;
}

int AnimatonWidgetScrollBar::frameToPos(int frame)
{
    return (frame - mMinFrame)*width()/(mMaxFrame - mMinFrame);
}

void AnimatonWidgetScrollBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.fillRect(rect(), QColor(200, 200, 200));

    QColor col;
    if(mPressed) {
        col = QColor(255, 75, 75);
    } else {
        col = QColor(200, 125, 125);
    }
    p.fillRect(frameToPos(mFirstViewedFrame), 0,
               mFramesSpan*width()/(mMaxFrame - mMinFrame), height(),
               col);

    p.end();
}

int clampInt2(int val, int min, int max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

void AnimatonWidgetScrollBar::wheelEvent(QWheelEvent *event)
{
    int newFramesSpan = mFramesSpan;
    if(event->delta() > 0) {
        newFramesSpan -= 2;
    } else {
        newFramesSpan += 2;
    }
    mFramesSpan = clampInt2(newFramesSpan, 50, 200);
    emitChange();
    repaint();
}

void AnimatonWidgetScrollBar::mousePressEvent(QMouseEvent *event)
{
    mPressed = true;
    mLastMouseEventFrame = posToFrame(event->x() );
    if(mLastMouseEventFrame < mFirstViewedFrame ||
            mLastMouseEventFrame > mFirstViewedFrame + mFramesSpan) {
        mFirstViewedFrame = mLastMouseEventFrame - mFramesSpan/2;
        emitChange();
    }
    mSavedFirstFrame = mFirstViewedFrame;
    repaint();
}

void AnimatonWidgetScrollBar::mouseMoveEvent(QMouseEvent *event)
{
    int newFrame = posToFrame(event->x() );
    int moveFrame = newFrame - mLastMouseEventFrame;
    if(moveFrame == 0) return;
    mFirstViewedFrame += moveFrame;
    emitChange();
    repaint();
    mLastMouseEventFrame = newFrame;
}

void AnimatonWidgetScrollBar::mouseReleaseEvent(QMouseEvent *)
{
    mPressed = false;
    repaint();
}

void AnimatonWidgetScrollBar::emitChange()
{
    emit viewedFramesChanged(mFirstViewedFrame,
                             mFirstViewedFrame + mFramesSpan);
}
