#ifndef ANIMATiONWIDGETSCROLLBAR_H
#define ANIMATiONWIDGETSCROLLBAR_H

#include <QWidget>
#include "sharedpointerdefs.h"
class CacheHandler;

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

    void setCacheHandler(CacheHandler *handler);
protected:
    qreal posToFrame(int xPos);
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
signals:
    void viewedFramesChanged(int, int);
public slots:
    void setViewedFramesRange(const int &startFrame,
                              const int &endFrame);

    void setMinMaxFrames(const int &minFrame,
                         const int &maxFrame);
private:
    CacheHandler* mCacheHandler_d = nullptr;
    bool mTopBorderVisible = true;
    bool mClamp;

    bool mRange;
    bool mPressed = false;
    QColor mHandleColor = QColor(100, 100, 100);
    qreal mLastMousePressFrame;
    int mSavedFirstFrame;

    int mFirstViewedFrame = 0;
    int mFramesSpan = 50;

    int mMinFrame = 0;
    int mMaxFrame = 200;

    int mMinSpan;
    int mMaxSpan;
    int mSpanInc;
};

#endif // ANIMATiONWIDGETSCROLLBAR_H
