#ifndef ANIMATiONWIDGETSCROLLBAR_H
#define ANIMATiONWIDGETSCROLLBAR_H

#include <QWidget>

class AnimationWidgetScrollBar : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationWidgetScrollBar(int minSpan, int maxSpan,
                                     int spanInc, int height, bool range,
                                     bool clamp,
                                     QWidget *parent = 0);
    void emitChange();
    bool setFirstViewedFrame(int firstFrame);
    void setFramesSpan(int newSpan);

    int getMaxFrame();
    int getMinFrame();
    int getFirstViewedFrame();
    int getLastViewedFrame();
    void setTopBorderVisible(const bool &bT);

    void setHandleColor(const QColor &col) {
        mHandleColor = col;
    }

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
    void setViewedFramesRange(int startFrame, int endFrame);

    void setMinMaxFrames(int minFrame, int maxFrame);
private:
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
