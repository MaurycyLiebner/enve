#include "animationwidgetscrollbar.h"
#include <QMouseEvent>
#include <QPainter>
#include "global.h"
#include "Colors/helpers.h"

AnimationWidgetScrollBar::AnimationWidgetScrollBar(const int &minSpan,
                                                   const int &maxSpan,
                                                   const int &spanInc,
                                                   const int &height,
                                                   const bool &range,
                                                   const bool &clamp,
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
    return (xPos - MIN_WIDGET_HEIGHT/2)*
            (mMaxFrame - mMinFrame + (mRange ? 0 : 1) ) /
            ((qreal)width() - 2*MIN_WIDGET_HEIGHT) + mMinFrame;
}

void AnimationWidgetScrollBar::setTopBorderVisible(const bool &bT) {
    mTopBorderVisible = bT;
}

void AnimationWidgetScrollBar::setCacheHandler(CacheHandler *handler) {
    mCacheHandler = handler;
}

#include "Boxes/rendercachehandler.h"
void AnimationWidgetScrollBar::paintEvent(QPaintEvent *) {
    QPainter p(this);

    p.fillRect(rect(), QColor(60, 60, 60));

    p.translate(MIN_WIDGET_HEIGHT/2, 0.);
    int dFrame = mMaxFrame - mMinFrame;
    if(!mRange) dFrame++;
    qreal pixPerFrame = ((qreal)width() - 2*MIN_WIDGET_HEIGHT)/dFrame;

    QColor col = mHandleColor;
    if(mPressed) {
        col.setHsv(col.hue(),
                   col.saturation(),
                   qMin(255, col.value() + 40));
    }

    p.fillRect(QRectF((mFirstViewedFrame - mMinFrame)*pixPerFrame, 0,
               mFramesSpan*pixPerFrame, height()), col);

    p.fillRect(-MIN_WIDGET_HEIGHT/2, 0,
               MIN_WIDGET_HEIGHT/2, height(),
               QColor(30, 30, 30));
    p.fillRect(width() - 2*MIN_WIDGET_HEIGHT, 0,
               2*MIN_WIDGET_HEIGHT - MIN_WIDGET_HEIGHT/2, height(),
               QColor(30, 30, 30));

    if(mCacheHandler != nullptr) {
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
    qreal maxX = width() + MIN_WIDGET_HEIGHT;
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
        p.drawText(QRectF(MIN_WIDGET_HEIGHT/2, 0,
                          5*MIN_WIDGET_HEIGHT, fullHeight),
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
    mFramesSpan = clampInt(newSpan, mMinSpan, mMaxSpan);
    if(mClamp) setFirstViewedFrame(mFirstViewedFrame);
}

int AnimationWidgetScrollBar::getMaxFrame() {
    return mMaxFrame;
}

int AnimationWidgetScrollBar::getMinFrame() {
    return mMinFrame;
}

void AnimationWidgetScrollBar::wheelEvent(QWheelEvent *event) {
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
    update();
}

bool AnimationWidgetScrollBar::setFirstViewedFrame(const int &firstFrame) {
    if(mClamp) {
        if(mRange) {
            mFirstViewedFrame = clampInt(firstFrame, mMinFrame, mMaxFrame -
                                          mFramesSpan);
        } else {
            mFirstViewedFrame = clampInt(firstFrame, mMinFrame, mMaxFrame);
        }
        return true;
    } else {
        mFirstViewedFrame = firstFrame;
        return true;
    }
    return false;
}

void AnimationWidgetScrollBar::mousePressEvent(QMouseEvent *event) {
    mPressed = true;
    mLastMousePressFrame = posToFrame(event->x() );
    if(mLastMousePressFrame < mFirstViewedFrame ||
            mLastMousePressFrame > mFirstViewedFrame + mFramesSpan) {
        setFirstViewedFrame(qRound(mLastMousePressFrame - mFramesSpan/2.));
        emitChange();
    }
    mSavedFirstFrame = mFirstViewedFrame;
    update();
}

void AnimationWidgetScrollBar::mouseMoveEvent(QMouseEvent *event) {
    qreal newFrame = posToFrame(event->x() );
    int moveFrame = qRound(newFrame - mLastMousePressFrame);
    if(setFirstViewedFrame(clampInt(mSavedFirstFrame + moveFrame,
                                     mMinFrame,
                                     mMaxFrame)) ) {
        emitChange();
        update();
    }
}

void AnimationWidgetScrollBar::mouseReleaseEvent(QMouseEvent *) {
    mPressed = false;
    update();
}

void AnimationWidgetScrollBar::setViewedFramesRange(const int &startFrame,
                                                    const int &endFrame) {
    setFirstViewedFrame(startFrame);
    setFramesSpan(endFrame - startFrame);
    update();
}

void AnimationWidgetScrollBar::setMinMaxFrames(const int &minFrame,
                                               const int &maxFrame) {
    mMinFrame = minFrame;
    mMaxFrame = maxFrame;
    mMaxSpan = mMaxFrame - mMinFrame;
    update();
}

void AnimationWidgetScrollBar::emitChange() {
    emit viewedFramesChanged(mFirstViewedFrame,
                             mFirstViewedFrame + mFramesSpan);
}

int AnimationWidgetScrollBar::getFirstViewedFrame() {
    return mFirstViewedFrame;
}

int AnimationWidgetScrollBar::getLastViewedFrame() {
    return mFirstViewedFrame + mFramesSpan;
}
