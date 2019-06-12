#include "animationwidgetscrollbar.h"
#include <QMouseEvent>
#include <QPainter>
#include "global.h"
#include "colorhelpers.h"

AnimationWidgetScrollBar::AnimationWidgetScrollBar(const int minSpan,
                                                   const int maxSpan,
                                                   const int spanInc,
                                                   const int height,
                                                   const bool range,
                                                   const bool clamp,
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

void AnimationWidgetScrollBar::setTopBorderVisible(const bool bT) {
    mTopBorderVisible = bT;
}

void AnimationWidgetScrollBar::setCurrentCanvas(Canvas * const canvas) {
    mCurrentCanvas = canvas;
}

void AnimationWidgetScrollBar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(60, 60, 60));

    const int dFrame = mMaxFrame - mMinFrame + (mRange ? 0 : 1);
    const qreal pixPerFrame = (width() - 2.*MIN_WIDGET_HEIGHT)/dFrame;

    const int f0 = -qCeil(0.5*MIN_WIDGET_HEIGHT/pixPerFrame);
    const int minFrame = mMinFrame + f0;
    const qreal x0 = f0*pixPerFrame + MIN_WIDGET_HEIGHT*0.5;

    const int f1 = qCeil(1.5*MIN_WIDGET_HEIGHT/pixPerFrame);
    const int maxFrame = mMaxFrame + f1;
    const qreal w1 = width() - 1.5*MIN_WIDGET_HEIGHT + f1*pixPerFrame - x0;

    QRect canvasMinRect;
    canvasMinRect.setLeft(qRound(x0));
    canvasMinRect.setTop(0);
    const int cRightFrames = mMinCanvasFrame - minFrame;
    canvasMinRect.setRight(qRound(x0 + cRightFrames*pixPerFrame));
    canvasMinRect.setBottom(height());
    p.fillRect(canvasMinRect, QColor(30, 30, 30));

    QRect canvasMaxRect;
    const int cLeftFrames = mMaxCanvasFrame - minFrame + (mRange ? 0 : 1);
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
        const int soundHeight = MIN_WIDGET_HEIGHT/3;
        const int rasterHeight = MIN_WIDGET_HEIGHT - soundHeight;
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
    const qreal maxX = width() + MIN_WIDGET_HEIGHT;
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
        const QRectF textRect(MIN_WIDGET_HEIGHT, 0,
                              5*MIN_WIDGET_HEIGHT, height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                   QString::number(mFirstViewedFrame));
    }

    p.setPen(QPen(Qt::black, 1));
    p.drawLine(0, height() - 1, width(), height() - 1);
    if(mTopBorderVisible) p.drawLine(0, 0, width(), 0);

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
        if(event->delta() > 0) newFramesSpan -= mSpanInc;
        else newFramesSpan += mSpanInc;
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

bool AnimationWidgetScrollBar::setFirstViewedFrame(const int firstFrame) {
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

void AnimationWidgetScrollBar::setDisplayedFrameRange(const int startFrame,
                                                      const int endFrame) {
    mMinFrame = startFrame;
    mMaxFrame = endFrame;
    mMaxSpan = mMaxFrame - mMinFrame;
    setViewedFrameRange(mFirstViewedFrame,
                        mFirstViewedFrame + mViewedFramesSpan);
}

void AnimationWidgetScrollBar::setViewedFrameRange(const int minFrame,
                                                   const int maxFrame) {
    setFirstViewedFrame(minFrame);
    setFramesSpan(maxFrame - minFrame);
    update();
}

void AnimationWidgetScrollBar::setCanvasFrameRange(const int minFrame,
                                                   const int maxFrame) {
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
