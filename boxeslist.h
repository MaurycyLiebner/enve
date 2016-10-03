#ifndef BOXESLIST_H
#define BOXESLIST_H

#include <QWidget>
#include "pointhelpers.h"

class AnimationWidget;

const qreal LIST_ITEM_HEIGHT = 20;
const qreal LIST_ITEM_MAX_WIDTH = 250;
const qreal LIST_ITEM_CHILD_INDENT = 20;

class QrealAnimator;

class QrealPoint;

class QrealKey;

class Canvas;

class MainWindow;

class BoxesList : public QWidget
{
    Q_OBJECT
public:
    explicit BoxesList(MainWindow *mainWindow, QWidget *parent = 0);

    int getMinViewedFrame();
    int getMaxViewedFrame();
    qreal getPixelsPerFrame();

    void updatePixelsPerFrame();

    static QPixmap *VISIBLE_PIXMAP;
    static QPixmap *INVISIBLE_PIXMAP;
    static QPixmap *HIDE_CHILDREN;
    static QPixmap *SHOW_CHILDREN;
    static QPixmap *LOCKED_PIXMAP;
    static QPixmap *UNLOCKED_PIXMAP;
    static QPixmap *ANIMATOR_CHILDREN_VISIBLE;
    static QPixmap *ANIMATOR_CHILDREN_HIDDEN;
    static QPixmap *ANIMATOR_RECORDING;
    static QPixmap *ANIMATOR_NOT_RECORDING;

    void addKeyToSelection(QrealKey *key);
    void removeKeyFromSelection(QrealKey *key);
    void clearKeySelection();
    void selectKeysInSelectionRect();
    bool processFilteredKeyEvent(QKeyEvent *event);

    // graph

    void graphPaint(QPainter *p);
    void graphWheelEvent(QWheelEvent *event);
    void graphMousePressEvent(QPoint eventPos, Qt::MouseButton eventButton);
    void graphMouseMoveEvent(QPoint eventPos, Qt::MouseButtons eventButtons);
    void graphMouseReleaseEvent(Qt::MouseButton eventButton);
    bool graphProcessFilteredKeyEvent(QKeyEvent *event);
    void graphResizeEvent(QResizeEvent *);
    void graphSetAnimator(QrealAnimator *animator);
    void graphIncScale(qreal inc);
    void graphSetScale(qreal scale);
    void graphUpdateDimensions();
    void graphIncMinShownVal(qreal inc);
    void graphSetMinShownVal(qreal newMinShownVal);
    void graphGetValueAndFrameFromPos(QPointF pos, qreal *value, qreal *frame);
    void graphMiddleMove(QPointF movePos);
    void graphMiddlePress(QPointF pressPos);
    void graphMouseRelease();
    void graphMousePress(QPointF pressPos);
    void graphMiddleRelease();
    void graphSetCtrlsModeForSelected(CtrlsMode mode);
    void graphDeletePressed();
    void graphClearKeysSelection();
    void graphAddKeyToSelection(QrealKey *key);
    void graphRemoveKeyFromSelection(QrealKey *key);
    void graphMouseMove(QPointF mousePos);
    void graphRepaint();
    void graphUpdateDrawPathIfNeeded();
    void graphSetDrawPathUpdateNeeded();
    void graphResetValueScaleAndMinShown();
    void ifIsCurrentAnimatorSetNull(QrealAnimator *animator);
    void middlePress(QPointF pressPos);
    void middleMove(QPointF movePos);
    void scheduleGraphUpdateAfterKeysChanged();
    void graphUpdateAfterKeysChangedIfNeeded();
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *e);
signals:
    void changedViewedFrames(int, int);
public slots:
    void graphUpdateAfterKeysChangedAndRepaint();

    void graphUpdateAfterKeysChanged();
    void graphMergeKeysIfNeeded();

    void setFramesRange(int startFrame, int endFrame);

    void graphSetSmoothCtrl();
    void graphSetSymmetricCtrl();
    void graphSetCornerCtrl();
    void graphSetTwoSideCtrlForSelected();
    void graphSetRightSideCtrlForSelected();
    void graphSetLeftSideCtrlForSelected();
    void graphSetNoSideCtrlForSelected();
private:
    void deleteSelectedKeys();

    bool mGraphUpdateAfterKeysChangedNeeded = false;

    QrealKey *mLastPressedKey = NULL;
    bool mFirstMove = false;
    int mMoveDFrame = 0;
    QPointF mLastPressPos;
    bool mMovingKeys = false;
    bool mSelecting = false;
    QRectF mSelectionRect;

    Canvas *mCanvas;
    QRectF mViewedRect;
    MainWindow *mMainWindow;
    QList<QrealKey*> mSelectedKeys;

    int mMinViewedFrame = 0;
    int mMaxViewedFrame = 50;
    qreal mPixelsPerFrame;


    // graph

    qreal mMargin = 20.;
    qreal mPixelsPerValUnit;
    qreal mMinShownVal;
    QrealPoint *mCurrentPoint = NULL;
    qreal mMinVal;
    qreal mMaxVal;
    qreal mValueScale = 1.;
    qreal mMinMoveFrame;
    qreal mMaxMoveFrame;
    QPointF mPressFrameAndValue;
    qreal mSavedMinViewedFrame;
    qreal mSavedMaxViewedFrame;
    qreal mSavedMinShownValue;
    QPointF mMiddlePressPos;
    QRectF mGraphRect;
    qreal mValueInc;

    QrealAnimator *mAnimator = NULL;

    bool mListFocus = false;
    bool mBeforePressListFocus = false;
};

#endif // BOXESLIST_H
