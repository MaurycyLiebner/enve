#include "animationwidgetscrollbar.h"
#include <QMouseEvent>
#include <QPainter>
#include "global.h"
#include "colorhelpers.h"

FrameScrollBar::FrameScrollBar(const int minSpan,
                                                   const int maxSpan,
                                                   const int height,
                                                   const bool range,
                                                   const bool clamp,
                                                   QWidget *parent) :
    QWidget(parent) {
    mMinSpan = minSpan;
    mMaxSpan = maxSpan;
    mRange = range;
    mClamp = clamp;
    setFramesSpan(0);

    setMinimumHeight(height);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
}

qreal FrameScrollBar::posToFrame(int xPos) {
    return (xPos - MIN_WIDGET_DIM/2)*
            (mFrameRange.fMax - mFrameRange.fMin + (mRange ? 0 : 1) ) /
            (qreal(width()) - 2*MIN_WIDGET_DIM) + mFrameRange.fMin;
}

void FrameScrollBar::setTopBorderVisible(const bool bT) {
    mTopBorderVisible = bT;
}

void FrameScrollBar::setCurrentCanvas(Canvas * const canvas) {
    mCurrentCanvas = canvas;
}

void FrameScrollBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(60, 60, 60));

    const int dFrame = mFrameRange.fMax - mFrameRange.fMin + (mRange ? 0 : 1);
    const qreal pixPerFrame = (width() - 2.*MIN_WIDGET_DIM)/dFrame;

    const int f0 = -qCeil(0.5*MIN_WIDGET_DIM/pixPerFrame);
    const int minFrame = mFrameRange.fMin + f0;
    const qreal x0 = f0*pixPerFrame + MIN_WIDGET_DIM*0.5;

    const int f1 = qCeil(1.5*MIN_WIDGET_DIM/pixPerFrame);
    const int maxFrame = mFrameRange.fMax + f1;
    const qreal w1 = width() - 1.5*MIN_WIDGET_DIM + f1*pixPerFrame - x0;

    QRect canvasMinRect;
    canvasMinRect.setLeft(qRound(x0));
    canvasMinRect.setTop(0);
    const int cRightFrames = mCanvasRange.fMin - minFrame;
    canvasMinRect.setRight(qRound(x0 + cRightFrames*pixPerFrame));
    canvasMinRect.setBottom(height());
    p.fillRect(canvasMinRect, QColor(30, 30, 30));

    QRect canvasMaxRect;
    const int cLeftFrames = mCanvasRange.fMax - minFrame + (mRange ? 0 : 1);
    const qreal left = cLeftFrames*pixPerFrame + x0;
    canvasMaxRect.setLeft(qMax(0, qRound(left)));
    canvasMaxRect.setTop(0);
    canvasMaxRect.setWidth(width());
    canvasMaxRect.setBottom(height());
    p.fillRect(canvasMaxRect, QColor(30, 30, 30));

    QColor col = mHandleColor;
    if(mPressed) {
        col.setHsv(col.hue(), col.saturation(), qMin(255, col.value() + 40));
    }
    QRect handleRect;
    const int hLeftFrames = mFirstViewedFrame - minFrame;
    handleRect.setLeft(qRound(hLeftFrames*pixPerFrame + x0));
    handleRect.setTop(0);
    handleRect.setWidth(qRound(mViewedFramesSpan*pixPerFrame));
    handleRect.setBottom(height());
    p.fillRect(handleRect, col);

    if(mCurrentCanvas) {
        const int soundHeight = MIN_WIDGET_DIM/3;
        const int rasterHeight = MIN_WIDGET_DIM - soundHeight;
        const QRectF rasterRect(x0, 0, w1, rasterHeight);
        const auto& rasterCache = mCurrentCanvas->getCacheHandler();
        rasterCache.drawCacheOnTimeline(&p, rasterRect, minFrame, maxFrame);

        const qreal fps = mCurrentCanvas->getFps();
        const QRectF soundRect(x0, rasterHeight, w1, soundHeight);
        const auto& soundCache = mCurrentCanvas->getSoundCacheHandler();
        soundCache.drawCacheOnTimeline(&p, soundRect, minFrame, maxFrame, fps);
    }

    p.setPen(Qt::white);

    int divInc = 3;
    int frameInc = 5000;
    while(frameInc && dFrame/frameInc < 3) {
        if(divInc == 3) {
            divInc = 0;
            frameInc *= 4;
            frameInc /= 10;
        } else {
            frameInc /= 2;
        }
        divInc++;
    }
    const qreal inc = frameInc*pixPerFrame;

    const int minMod = minFrame%frameInc;
    qreal xL = (-minMod + (mRange ? 0. : 0.5))*pixPerFrame + x0;
    int currentFrame = minFrame - minMod;
    if(!mRange) {
        while(xL < 70) {
            currentFrame += frameInc;
            xL += inc;
        }
    }

    const qreal halfHeight = height()*0.5;
    const qreal quarterHeight = height()*0.25;
    const qreal threeFourthsHeight = height()*0.75;
    const qreal maxX = width() + MIN_WIDGET_DIM;
    while(xL < maxX) {
        p.drawText(QRectF(xL - inc, quarterHeight, 2*inc, halfHeight),
                   Qt::AlignCenter, QString::number(currentFrame));
        p.drawLine(QPointF(xL, threeFourthsHeight + 2),
                   QPointF(xL, height()));
        xL += inc;
        currentFrame += frameInc;
    }

    p.setPen(QPen(Qt::white, 1));
    if(!mRange) {
        const QRectF textRect(MIN_WIDGET_DIM, 0,
                              5*MIN_WIDGET_DIM, height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                   QString::number(mFirstViewedFrame));
    }

    p.setPen(QPen(Qt::black, 1));
    p.drawLine(0, height() - 1, width(), height() - 1);
    if(mTopBorderVisible) p.drawLine(0, 0, width(), 0);

    p.end();
}

void FrameScrollBar::setFramesSpan(int newSpan) {
    mViewedFramesSpan = clampInt(newSpan, mMinSpan, mMaxSpan);
    if(mClamp) setFirstViewedFrame(mFirstViewedFrame);
}

int FrameScrollBar::getMaxFrame() {
    return mFrameRange.fMax;
}

int FrameScrollBar::getMinFrame() {
    return mFrameRange.fMin;
}

void FrameScrollBar::wheelEvent(QWheelEvent *event) {
    if(mRange) {
        if(event->modifiers() & Qt::CTRL) {
            if(event->delta() > 0) {
                setFirstViewedFrame(mFirstViewedFrame - mViewedFramesSpan/20);
            } else {
                setFirstViewedFrame(mFirstViewedFrame + mViewedFramesSpan/20);
            }
        } else {
            int newFramesSpan = mViewedFramesSpan;
            if(event->delta() > 0) newFramesSpan *= 0.85;
            else newFramesSpan *= 1.15;
            setFramesSpan(newFramesSpan);
        }
    } else {
        if(event->delta() > 0) {
            setFirstViewedFrame(mFirstViewedFrame - 1);
        } else {
            setFirstViewedFrame(mFirstViewedFrame + 1);
        }
    }

    emitChange();
    update();
}

bool FrameScrollBar::setFirstViewedFrame(const int firstFrame) {
    if(mClamp) {
        if(mRange) {
            mFirstViewedFrame = clampInt(firstFrame, mFrameRange.fMin, mFrameRange.fMax -
                                          mViewedFramesSpan);
        } else {
            mFirstViewedFrame = clampInt(firstFrame, mFrameRange.fMin, mFrameRange.fMax);
        }
        return true;
    } else {
        mFirstViewedFrame = firstFrame;
        return true;
    }
}
#include <QMenu>
void FrameScrollBar::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        QMenu menu(this);
        QAction *clampAction = new QAction("Clamp", this);
        clampAction->setCheckable(true);
        clampAction->setChecked(mClamp);
        menu.addAction(clampAction);

        menu.addSeparator();

        QAction *timeAction = new QAction("Display Time", this);
        timeAction->setChecked(mDisplayTime);
        menu.addAction(timeAction);

        QAction *framesAction = new QAction("Display Frames", this);
        framesAction->setChecked(!mDisplayTime);
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

void FrameScrollBar::mouseMoveEvent(QMouseEvent *event) {
    qreal newFrame = posToFrame(event->x() );
    int moveFrame = qRound(newFrame - mLastMousePressFrame);
    if(setFirstViewedFrame(mSavedFirstFrame + moveFrame)) {
        emitChange();
        update();
    }
}

void FrameScrollBar::mouseReleaseEvent(QMouseEvent *) {
    mPressed = false;
    update();
}

void FrameScrollBar::setDisplayedFrameRange(const FrameRange& range) {
    mFrameRange = range;
    mMaxSpan = range.span() - 1;
    setViewedFrameRange({mFirstViewedFrame,
                         mFirstViewedFrame + mViewedFramesSpan});
}

void FrameScrollBar::setViewedFrameRange(const FrameRange& range) {
    setFirstViewedFrame(range.fMin);
    setFramesSpan(range.span() - 1);
    update();
}

void FrameScrollBar::setCanvasFrameRange(const FrameRange &range) {
    mCanvasRange = range;
    update();
}

void FrameScrollBar::emitChange() {
    emit viewedFrameRangeChanged(getViewedRange());
}

FrameRange FrameScrollBar::getViewedRange() const {
    return {mFirstViewedFrame, getLastViewedFrame()};
}

int FrameScrollBar::getFirstViewedFrame() const {
    return mFirstViewedFrame;
}

int FrameScrollBar::getLastViewedFrame() const {
    return mFirstViewedFrame + mViewedFramesSpan;
}
