#ifndef KEYSVIEW_H
#define KEYSVIEW_H

#include <QWidget>
#include <QPointer>
#include "keyfocustarget.h"
#include "smartPointers/sharedpointerdefs.h"
#include "framerange.h"

const QList<QColor> ANIMATOR_COLORS = {QColor(255, 0, 0) , QColor(0, 255, 255),
                                      QColor(255, 255, 0), QColor(255, 0, 255),
                                      QColor(0, 255, 0)};

class Key;
class QrealKey;
class QrealAnimator;
class GraphAnimator;
class QrealPoint;
class Canvas;
class MainWindow;
class BoxScrollWidgetVisiblePart;
class AnimationDockWidget;
class DurationRectangleMovable;
class KeysClipboardContainer;
class Animator;
#include "valueinput.h"
enum CtrlsMode : short;

class KeysView : public QWidget, public KeyFocusTarget {
    Q_OBJECT
public:
    explicit KeysView(BoxScrollWidgetVisiblePart *boxesListVisible,
                      QWidget *parent = nullptr);

    void setCurrentScene(Canvas* const scene);

    void setGraphViewed(const bool bT);

    int getMinViewedFrame();
    int getMaxViewedFrame();
    qreal getPixelsPerFrame();

    void updatePixelsPerFrame();

    void addKeyToSelection(Key * const key);
    void removeKeyFromSelection(Key * const key);
    void clearKeySelection();
    void selectKeysInSelectionRect();

    // graph

    void graphPaint(QPainter *p);
    void graphWheelEvent(QWheelEvent *event);
    bool graphProcessFilteredKeyEvent(QKeyEvent *event);
    void graphResizeEvent(QResizeEvent *);
    void graphAddViewedAnimator(GraphAnimator * const animator);
    void graphIncScale(const qreal inc);
    void graphSetScale(const qreal scale);
    void graphUpdateDimensions();
    void graphIncMinShownVal(const qreal inc);
    void graphSetMinShownVal(const qreal newMinShownVal);
    void graphGetValueAndFrameFromPos(const QPointF &pos,
                                      qreal &value, qreal &frame) const;
    void graphMiddleMove(const QPointF &movePos);
    void gMiddlePress(const QPointF &pressPos);
    void gMouseRelease();
    void graphMousePress(const QPointF &pressPos);
    void graphMiddleRelease();
    void graphSetCtrlsModeForSelected(const CtrlsMode &mode);
    void graphDeletePressed();
    void graphResetValueScaleAndMinShown();
    void scheduleGraphUpdateAfterKeysChanged();
    void graphUpdateAfterKeysChangedIfNeeded();
    void deleteSelectedKeys();
    void middleMove(const QPointF &movePos);
    void middlePress(const QPointF &pressPos);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *e);
    void graphRemoveViewedAnimator(GraphAnimator * const animator);
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

    void leaveEvent(QEvent *) {
        clearHovered();
        update();
    }

    void KFT_setFocusToWidget() {
        setFocus();
        update();
    }

    void KFT_clearFocus() {
        clearFocus();
        update();
    }
signals:
    void changedViewedFrames(FrameRange);
    void wheelEventSignal(QWheelEvent*);
public:
    static QColor sGetAnimatorColor(const int i);

    void graphResetValueScaleAndMinShownAction();
    void graphUpdateAfterKeysChangedAndRepaint();

    void graphUpdateAfterKeysChanged();

    void setFramesRange(const FrameRange &range);

    void graphMakeSegmentsLinearAction();
    void graphMakeSegmentsSmoothAction();
    void graphSetSmoothCtrlAction();
    void graphSetSymmetricCtrlAction();
    void graphSetCornerCtrlAction();
    void graphSetTwoSideCtrlForSelected();

    void graphClearAnimatorSelection();

    void setViewedVerticalRange(const int top, const int bottom);
    void clearHovered();

    int graphGetAnimatorId(GraphAnimator * const anim);
    QrealPoint *graphGetPointAtPos(const QPointF &pressPos) const;
private:
    void scrollRight();
    void scrollLeft();
    void graphConstrainAnimatorCtrlsFrameValues();
    void graphGetAnimatorsMinMaxValue(qreal &minVal, qreal &maxVal);
    void graphMakeSegmentsSmoothAction(const bool smooth);
    void sortSelectedKeys();
    void clearHoveredKey();

    stdsptr<KeysClipboardContainer> getSelectedKeysClipboardContainer();

    QTimer *mScrollTimer;

    void updateHovered(const QPoint &posU);

    QPointer<DurationRectangleMovable> mHoveredMovable;
    stdptr<Key> mHoveredKey;

    void grabMouseAndTrack() {
        mIsMouseGrabbing = true;
        grabMouse();
    }

    void releaseMouseAndDontTrack() {
        mIsMouseGrabbing = false;
        releaseMouse();
    }

    bool mIsMouseGrabbing = false;
    int mViewedTop = 0;
    int mViewedBottom = 0;

    BoxScrollWidgetVisiblePart *mBoxesListVisible;
    QRectF mSelectionRect;
    bool mSelecting = false;
    bool mGraphViewed = false;
    qreal mPixelsPerFrame;
    QPointF mMiddlePressPos;

    bool mGraphUpdateAfterKeysChangedNeeded = false;

    QPointer<DurationRectangleMovable> mLastPressedMovable;
    Key *mLastPressedKey = nullptr;
    bool mFirstMove = false;
    int mMoveDFrame = 0;
    QPoint mLastPressPos;
    bool mMovingKeys = false;
    bool mScalingKeys = false;
    bool mMovingRect = false;
    bool mPressedCtrlPoint = false;

    MainWindow *mMainWindow;
    Canvas* mCurrentScene = nullptr;
    QList<Animator*> mSelectedKeysAnimators;
    QList<GraphAnimator*> mGraphAnimators;

    int mMinViewedFrame = 0;
    int mMaxViewedFrame = 50;

    int mSavedMinViewedFrame = 0;
    int mSavedMaxViewedFrame = 0;

    // graph

    qreal mPixelsPerValUnit = 0;
    qreal mMinShownVal = 0;
    stdptr<QrealPoint> mGHoveredPoint;
    stdptr<QrealPoint> mGPressedPoint;
    qreal mMinMoveVal = 0;
    qreal mMaxMoveVal = 0;
    qreal mMinMoveFrame = 0;
    qreal mMaxMoveFrame = 0;
    qreal mSavedMinShownValue = 0;
    qreal mValueInc = 0;
    int mValuePrec = 2;
};

#endif // KEYSVIEW_H
