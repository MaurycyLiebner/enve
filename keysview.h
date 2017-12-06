#ifndef KEYSVIEW_H
#define KEYSVIEW_H

#include <QWidget>
#include <QPointer>
#include "keyfocustarget.h"

const QList<QColor> ANIMATORCOLORS = {QColor(255, 0, 0) , QColor(0, 255, 255),
                                      QColor(255, 255, 0), QColor(255, 0, 255),
                                      QColor(0, 255, 0)};

class Key;
class QrealKey;
class QrealAnimator;
class QrealPoint;
class Canvas;
class MainWindow;
class BoxScrollWidgetVisiblePart;
class AnimationDockWidget;
class DurationRectangleMovable;
#include "valueinput.h"
enum CtrlsMode : short;

class KeysView : public QWidget, public KeyFocusTarget {
    Q_OBJECT
public:
    explicit KeysView(BoxScrollWidgetVisiblePart *boxesListVisible,
                      QWidget *parent = 0);

    void setGraphViewed(bool bT);

    int getMinViewedFrame();
    int getMaxViewedFrame();
    qreal getPixelsPerFrame();

    void updatePixelsPerFrame();

    void addKeyToSelection(Key *key);
    void removeKeyFromSelection(Key *key);
    void clearKeySelection();
    void selectKeysInSelectionRect();

    // graph

    void graphPaint(QPainter *p);
    void graphWheelEvent(QWheelEvent *event);
    void graphMousePressEvent(const QPoint &eventPos,
                              const Qt::MouseButton &eventButton);
    void graphMouseMoveEvent(const QPoint &eventPos,
                             const Qt::MouseButtons &eventButtons);
    void graphMouseReleaseEvent(const Qt::MouseButton &eventButton);
    bool graphProcessFilteredKeyEvent(QKeyEvent *event);
    void graphResizeEvent(QResizeEvent *);
    void graphAddViewedAnimator(QrealAnimator *animator);
    void graphIncScale(const qreal &inc);
    void graphSetScale(const qreal &scale);
    void graphUpdateDimensions();
    void graphIncMinShownVal(const qreal &inc);
    void graphSetMinShownVal(const qreal &newMinShownVal);
    void graphGetValueAndFrameFromPos(const QPointF &pos,
                                      qreal *value, qreal *frame);
    void graphMiddleMove(const QPointF &movePos);
    void graphMiddlePress(const QPointF &pressPos);
    void graphMouseRelease();
    void graphMousePress(const QPointF &pressPos);
    void graphMiddleRelease();
    void graphSetCtrlsModeForSelected(const CtrlsMode &mode);
    void graphDeletePressed();
    void graphClearKeysSelection();
    void graphAddKeyToSelection(QrealKey *key);
    void graphRemoveKeyFromSelection(QrealKey *key);
    void graphMouseMove(const QPointF &mousePos);
    void graphResetValueScaleAndMinShown();
    void scheduleGraphUpdateAfterKeysChanged();
    void graphUpdateAfterKeysChangedIfNeeded();
    void deleteSelectedKeys();
    void middleMove(const QPointF &movePos);
    void middlePress(const QPointF &pressPos);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *e);
    void graphRemoveViewedAnimator(QrealAnimator *animator);
    void updateAnimatorsColors();
    void clearHoveredMovable();
    bool KFT_handleKeyEventForTarget(QKeyEvent *event);
    void handleMouseMove(const QPoint &pos,
                         const Qt::MouseButtons &buttons);
protected:
    ValueInput mValueInput;
    QPoint mLastMovePos;
    void focusInEvent(QFocusEvent *);

    void resizeEvent(QResizeEvent *e);

    void wheelEvent(QWheelEvent *e);

    void mousePressEvent(QMouseEvent *e);

    void paintEvent(QPaintEvent *);

    void KFT_setFocusToWidget() {
        setFocus();
    }

signals:
    void changedViewedFrames(int, int);
    void wheelEventSignal(QWheelEvent*);
public slots:
    void graphResetValueScaleAndMinShownAction();
    void graphUpdateAfterKeysChangedAndRepaint();

    void graphUpdateAfterKeysChanged();
    void graphMergeKeysIfNeeded();

    void setFramesRange(const int &startFrame,
                        const int &endFrame);

    void graphSetSmoothCtrl();
    void graphSetSymmetricCtrl();
    void graphSetCornerCtrl();
    void graphSetTwoSideCtrlForSelected();
    void graphSetRightSideCtrlForSelected();
    void graphSetLeftSideCtrlForSelected();
    void graphSetNoSideCtrlForSelected();

    void graphClearAnimatorSelection();

    void setViewedRange(const int &top,
                        const int &bottom);
private slots:
    void scrollRight();
    void scrollLeft();
private:
    QTimer *mScrollTimer;

    void updateHoveredPointFromPos(const QPoint &posU);

    QPointer<DurationRectangleMovable> mHoveredMovable;
    Key *mHoveredKey = NULL;

    bool mIsMouseGrabbing = false;
    int mViewedTop = 0;
    int mViewedBottom = 0;

    BoxScrollWidgetVisiblePart *mBoxesListVisible;
    QRectF mSelectionRect;
    bool mSelecting = false;
    bool mGraphViewed = false;
    qreal mPixelsPerFrame;
    QPointF mMiddlePressPos;
    QList<QPointer<QrealAnimator>> mAnimators;

    bool mGraphUpdateAfterKeysChangedNeeded = false;

    QPointer<DurationRectangleMovable> mLastPressedDurationRectangleMovable;
    Key *mLastPressedKey = NULL;
    bool mFirstMove = false;
    int mMoveDFrame = 0;
    QPointF mLastPressPos;
    bool mMovingKeys = false;
    bool mScalingKeys = false;
    bool mMovingRect = false;

    MainWindow *mMainWindow;
    QList<Key*> mSelectedKeys;

    int mMinViewedFrame = 0;
    int mMaxViewedFrame = 50;


    // graph

    qreal mPixelsPerValUnit = 0.;
    qreal mMinShownVal = 0.;
    QrealPoint *mCurrentPoint = NULL;
    qreal mMinVal = 0.;
    qreal mMaxVal = 0.;
    qreal mMinMoveFrame = 0.;
    qreal mMaxMoveFrame = 0.;
    QPointF mPressFrameAndValue = QPointF(0., 0.);
    qreal mSavedMinViewedFrame = 0.;
    qreal mSavedMaxViewedFrame = 0.;
    qreal mSavedMinShownValue = 0.;
    qreal mValueInc = 0.;
    int mValuePrec = 2;
    void graphConstrainAnimatorCtrlsFrameValues();
    void graphGetAnimatorsMinMaxValue(qreal *minVal, qreal *maxVal);
    void clearHoveredPoint();
};

#endif // KEYSVIEW_H
