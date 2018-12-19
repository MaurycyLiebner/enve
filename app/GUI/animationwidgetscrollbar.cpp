#include "animationwidgetscrollbar.h"
#include <QMouseEvent>
#include <QPainter>
#include "global.h"
#include "colorhelpers.h"

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
    mCacheHandler_d = handler;
}

#include "Boxes/rendercachehandler.h"
void AnimationWidgetScrollBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.save();
    p.fillRect(rect(), QColor(60, 60, 60));

    int dFrame = mMaxFrame - mMinFrame;
    if(!mRange) dFrame++;
    qreal pixPerFrame = ((qreal)width() - 2*MIN_WIDGET_HEIGHT)/dFrame;
    p.translate(MIN_WIDGET_HEIGHT/2, 0.);

    int canvasMinX = (mMinCanvasFrame - mMinFrame)*pixPerFrame;
    int canvasMaxX = (mMaxCanvasFrame - mMinFrame + (mRange ? 0 : 1))*pixPerFrame;

    if(canvasMinX > -1) {
        p.fillRect(-MIN_WIDGET_HEIGHT, 0,
                   canvasMinX + MIN_WIDGET_HEIGHT, height(),
                   QColor(30, 30, 30));
    }
    if(canvasMaxX < width()) {
        p.fillRect(canvasMaxX, 0,
                   width() - canvasMaxX + MIN_WIDGET_HEIGHT, height(),
                   QColor(30, 30, 30));
    }

    QColor col = mHandleColor;
    if(mPressed) {
        col.setHsv(col.hue(),
                   col.saturation(),
                   qMin(255, col.value() + 40));
    }

    p.fillRect(QRectF((mFirstViewedFrame - mMinFrame)*pixPerFrame, 0,
               mViewedFramesSpan*pixPerFrame, height()), col);

//    p.fillRect(-MIN_WIDGET_HEIGHT/2, 0,
//               MIN_WIDGET_HEIGHT/2, height(),
//               QColor(30, 30, 30));
//    p.fillRect(width() - 2*MIN_WIDGET_HEIGHT, 0,
//               2*MIN_WIDGET_HEIGHT - MIN_WIDGET_HEIGHT/2, height(),
//               QColor(30, 30, 30));

    if(mCacheHandler_d != nullptr) {
        mCacheHandler_d->drawCacheOnTimeline(&p, pixPerFrame, 0.,
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

    p.restore();
    p.setPen(QPen(Qt::black, 1.));
    p.drawLine(0, height() - 1, width(), height() - 1);
    if(mTopBorderVisible) {
        p.drawLine(0, 0, width(), 0);
    }

    p.end();
}

void AnimationWidgetScrollBar::setFramesSpan(int newSpan) {
    mViewedFramesSpan = clampInt(newSpan, mMinSpan, mMaxSpan);
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
        int newFramesSpan = mViewedFramesSpan;
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
                                          mViewedFramesSpan);
        } else {
            mFirstViewedFrame = clampInt(firstFrame, mMinFrame, mMaxFrame);
        }
        return true;
    } else {
        mFirstViewedFrame = firstFrame;
        return true;
    }
}
#include <QMenu>
void AnimationWidgetScrollBar::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        QMenu menu(this);
        QAction *clampAction = new QAction("Clamp", this);
        clampAction->setCheckable(true);
        clampAction->setChecked(mClamp);
        menu.addAction(clampAction);

        menu.addSeparator();

        QAction *timeAction = new QAction("Display Time", this);
        timeAction->setEnabled(!mDisplayTime);
        menu.addAction(timeAction);

        QAction *framesAction = new QAction("Display Frames", this);
        framesAction->setEnabled(mDisplayTime);
        menu.addAction(framesAction);

        QAction* selectedAction = menu.exec(event->globalPos());
        if(selectedAction) {
            if(selectedAction == clampAction) {
                mClamp = !mClamp;
            } else if(selectedAction == framesAction) {
                mDisplayTime = false;
            } else if(selectedAction == timeAction) {
                mDisplayTime = true;
            }
        }
        return;
    }
    mPressed = true;
    mLastMousePressFrame = posToFrame(event->x() );
    if(mLastMousePressFrame < mFirstViewedFrame ||
            mLastMousePressFrame > mFirstViewedFrame + mViewedFramesSpan) {
        setFirstViewedFrame(qRound(mLastMousePressFrame - mViewedFramesSpan/2.));
        emitChange();
    }
    mSavedFirstFrame = mFirstViewedFrame;
    update();
}

void AnimationWidgetScrollBar::mouseMoveEvent(QMouseEvent *event) {
    qreal newFrame = posToFrame(event->x() );
    int moveFrame = qRound(newFrame - mLastMousePressFrame);
    if(setFirstViewedFrame(mSavedFirstFrame + moveFrame)) {
        emitChange();
        update();
    }
}

void AnimationWidgetScrollBar::mouseReleaseEvent(QMouseEvent *) {
    mPressed = false;
    update();
}

void AnimationWidgetScrollBar::setDisplayedFrameRange(const int &startFrame,
                                                      const int &endFrame) {
    mMinFrame = startFrame;
    mMaxFrame = endFrame;
    mMaxSpan = mMaxFrame - mMinFrame;
    setViewedFrameRange(mFirstViewedFrame,
                        mFirstViewedFrame + mViewedFramesSpan);
}

void AnimationWidgetScrollBar::setViewedFrameRange(const int &minFrame,
                                                   const int &maxFrame) {
    setFirstViewedFrame(minFrame);
    setFramesSpan(maxFrame - minFrame);
    update();
}

void AnimationWidgetScrollBar::setCanvasFrameRange(const int &minFrame,
                                                   const int &maxFrame) {
    mMinCanvasFrame = minFrame;
    mMaxCanvasFrame = maxFrame;
    update();
}

void AnimationWidgetScrollBar::emitChange() {
    emit viewedFrameRangeChanged(mFirstViewedFrame,
                             mFirstViewedFrame + mViewedFramesSpan);
}

int AnimationWidgetScrollBar::getFirstViewedFrame() {
    return mFirstViewedFrame;
}

int AnimationWidgetScrollBar::getLastViewedFrame() {
    return mFirstViewedFrame + mViewedFramesSpan;
}
