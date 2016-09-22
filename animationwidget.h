#ifndef ANIMATIONWIDGET_H
#define ANIMATIONWIDGET_H

#include <QWidget>
#include "qrealanimator.h"

class BoxesList;

class AnimationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationWidget(QWidget *parent, MainWindow *window);
    bool processFilteredKeyEvent(QKeyEvent *event);

    void setAnimator(QrealAnimator *animator);

    void setBoxesList(BoxesList *boxesList);

    void repaintWithBoxesList();


    void getValueAndFrameFromPos(QPointF pos, qreal *value, qreal *frame);
    void mousePress(QPointF pressPos);
    void mouseMove(QPointF mousePos);
    void clearKeysSelection();
    void addKeyToSelection(QrealKey *key);
    void removeKeyFromSelection(QrealKey *key);
    void updateDimensions();
    void incScale(qreal inc);
    void setScale(qreal scale);
    void incMinShownVal(qreal inc);
    void setMinShownVal(qreal newMinShownVal);
    void middlePress(QPointF pressPos);
    void middleMove(QPointF movePos);
    void middleRelease();
    void mouseRelease();
    void setCtrlsModeForSelected(CtrlsMode mode);
    void deletePressed();
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
signals:
    void changedViewedFrames(int, int);
public slots:
    void mergeKeysIfNeeded();
    void updateDrawPath();
    void setViewedFramesRange(int startFrame, int endFrame);
    void setCurrentFrame(int frame);

    void setTwoSideCtrlForSelected();
    void setRightSideCtrlForSelected();
    void setLeftSideCtrlForSelected();
    void setNoSideCtrlForSelected();
    void setSmoothCtrl();
    void setSymmetricCtrl();
    void setCornerCtrl();
private:
    MainWindow *mMainWindow;
    int mCurrentFrame;
    bool mFirstMove = true;
    qreal mMargin = 20.;
    qreal mPixelsPerValUnit;
    qreal mPixelsPerFrame;
    int mStartFrame;
    int mEndFrame;
    qreal mMinShownVal;
    QrealPoint *mCurrentPoint = NULL;
    QList<QrealKey*> mSelectedKeys;
    qreal mMinVal;
    qreal mMaxVal;
    qreal mValueScale = 1.;
    qreal mMinMoveFrame;
    qreal mMaxMoveFrame;
    bool mSelecting = false;
    QRectF mSelectionRect;
    QPointF mPressFrameAndValue;
    qreal mSavedStartFrame;
    qreal mSavedEndFrame;
    qreal mSavedMinShownValue;
    QPointF mMiddlePressPos;

    BoxesList *mBoxesList = NULL;
    QrealAnimator *mAnimator = NULL;
};
#endif // ANIMATIONWIDGET_H
