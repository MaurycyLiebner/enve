#ifndef ANIMATiONWIDGETSCROLLBAR_H
#define ANIMATiONWIDGETSCROLLBAR_H

#include <QWidget>
#include "smartPointers/sharedpointerdefs.h"
#include "canvas.h"
class HDDCachableCacheHandler;

class FrameScrollBar : public QWidget {
    Q_OBJECT
public:
    explicit FrameScrollBar(const int minSpan,
                            const int maxSpan,
                            const int height,
                            const bool range,
                            const bool clamp,
                            QWidget *parent = nullptr);
    void emitChange();
    bool setFirstViewedFrame(const int firstFrame);
    void setFramesSpan(int newSpan);

    int getMaxFrame();
    int getMinFrame();

    FrameRange getViewedRange() const;
    int getFirstViewedFrame() const;
    int getLastViewedFrame() const;

    void setHandleColor(const QColor &col) {
        mHandleColor = col;
    }

    void setCurrentCanvas(Canvas * const canvas);

    void setDisplayedFrameRange(const FrameRange& range);
    void setViewedFrameRange(const FrameRange& range);
    void setCanvasFrameRange(const FrameRange& range);
protected:
    qreal posToFrame(int xPos);
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
signals:
    void viewedFrameRangeChanged(FrameRange);
private:
    bool mClamp;
    bool mDisplayTime = false;
    bool mRange;
    bool mPressed = false;

    int mSavedFirstFrame;

    int mFirstViewedFrame = 0;
    int mViewedFramesSpan = 50;

    FrameRange mFrameRange{0, 200};

    int mMinSpan;
    int mMaxSpan;
    int mSpanInc;

    FrameRange mCanvasRange{0, 200};

    qreal mFps;
    qreal mLastMousePressFrame;

    QColor mHandleColor = QColor(100, 100, 100);
    Canvas* mCurrentCanvas = nullptr;
};

#endif // ANIMATiONWIDGETSCROLLBAR_H
