#ifndef ANIMATiONWIDGETSCROLLBAR_H
#define ANIMATiONWIDGETSCROLLBAR_H

#include <QWidget>
#include "smartPointers/sharedpointerdefs.h"
#include "canvas.h"
class RenderCacheHandler;

class AnimationWidgetScrollBar : public QWidget {
    Q_OBJECT
public:
    explicit AnimationWidgetScrollBar(const int &minSpan,
                                      const int &maxSpan,
                                      const int &spanInc,
                                      const int &height,
                                      const bool &range,
                                      const bool &clamp,
                                      QWidget *parent = nullptr);
    virtual ~AnimationWidgetScrollBar() {}
    void emitChange();
    bool setFirstViewedFrame(const int &firstFrame);
    void setFramesSpan(int newSpan);

    int getMaxFrame();
    int getMinFrame();
    int getFirstViewedFrame();
    int getLastViewedFrame();
    void setTopBorderVisible(const bool &bT);

    void setHandleColor(const QColor &col) {
        mHandleColor = col;
    }

    void setCurrentCanvas(Canvas * const canvas);

    void setDisplayedFrameRange(const int &minFrame,
                                const int &maxFrame);
    void setViewedFrameRange(const int &minFrame,
                             const int &maxFrame);
    void setCanvasFrameRange(const int &minFrame,
                             const int &maxFrame);
protected:
    qreal posToFrame(int xPos);
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
signals:
    void viewedFrameRangeChanged(int, int);
private:
    bool mTopBorderVisible = true;
    bool mClamp;
    bool mDisplayTime = false;
    bool mRange;
    bool mPressed = false;

    int mSavedFirstFrame;

    int mFirstViewedFrame = 0;
    int mViewedFramesSpan = 50;

    int mMinFrame = 0;
    int mMaxFrame = 200;

    int mMinSpan;
    int mMaxSpan;
    int mSpanInc;

    int mMinCanvasFrame = 0;
    int mMaxCanvasFrame = 200;

    qreal mFps;
    qreal mLastMousePressFrame;

    QColor mHandleColor = QColor(100, 100, 100);
    qptr<Canvas> mCurrentCanvas;
};

#endif // ANIMATiONWIDGETSCROLLBAR_H
