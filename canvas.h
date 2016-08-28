#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include "boxesgroup.h"
#include "Colors/color.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>

class MainWindow;

class UndoRedo;

class UndoRedoStack;

enum CanvasMode : short {
    MOVE_PATH,
    MOVE_POINT,
    ADD_POINT,
    PICK_PATH_SETTINGS
};

class Canvas : public QWidget, public BoxesGroup
{
    Q_OBJECT
public:
    explicit Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
                    MainWindow *parent = 0);
    QRectF getBoundingRect();
    void addBoxToSelection(BoundingBox *box);
    void clearBoxesSelection();
    void selectOnlyLastPressedBox();
    void removePointFromSelection(MovablePoint *point);
    void removeBoxFromSelection(BoundingBox *box);
    void selectOnlyLastPressedPoint();
    void connectPointsFromDifferentPaths(PathPoint *pointSrc, PathPoint *pointDest);

    void scheduleRepaint();

    void repaintIfNeeded();
    void setCanvasMode(CanvasMode mode);
    void startSelectionAtPoint(QPointF pos);
    void moveSecondSelectionPoint(QPointF pos);
    void clearAllPointsSelection();
    void clearAllPathsSelection();
    void setPointCtrlsMode(CtrlsMode mode);
    QPointF scaleDistancePointByCurrentScale(QPointF point);
    void setCurrentBoxesGroup(BoxesGroup *group);

    void rotateBoxesBy(qreal rotChange, QPointF absOrigin, bool startTrans);
    void updatePivot();

    void schedulePivotUpdate();
    void updatePivotIfNeeded();
    void setPivotPositionForSelected();
    void scaleBoxesBy(qreal scaleBy, QPointF absOrigin, bool startTrans);

    void saveToSql();
    void loadAllBoxesFromSql(bool loadInBox);
    void clearAll();
    void resetTransormation();
    void fitCanvasToSize();
    void saveSelectedToSqlForCurrentBox();
    bool processFilteredKeyEvent(QKeyEvent *event);
    void scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin);
    void scale(qreal scaleBy, QPointF absOrigin);
    void pickPathForSettings();
    void moveBy(QPointF trans);
protected:
//    void updateAfterCombinedTransformationChanged();
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void clearPointsSelection();
    void addPointToSelection(MovablePoint *point);

    void setCurrentEndPoint(PathPoint *point);

    PathPoint *getCurrentPoint();

    void handleMovePathMouseRelease(QPointF pos);
    void handleMovePointMouseRelease(QPointF pos);

    bool isMovingPath();

    qreal getCurrentCanvasScale();
signals:
private slots:
    void fillSettingsChanged(PaintSettings fillSettings, bool saveUndoRedo);
    void strokeSettingsChanged(StrokeSettings strokeSettings, bool saveUndoRedo);

    void startStrokeSettingsTransform();
    void startFillSettingsTransform();
    void finishStrokeSettingsTransform();
    void finishFillSettingsTransform();
public slots:
    void connectPointsSlot();
    void disconnectPointsSlot();
    void mergePointsSlot();

    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();
private:    
    bool mCancelTransform = false;
    bool mIsMouseGrabbing = false;

    QImage mBgImage;

    bool mDoubleClick = false;
    int mMovesToSkip = 0;

    Color mFillColor;
    Color mOutlineColor;

    BoxesGroup *mCurrentBoxesGroup;

    int mWidth = 1920;
    int mHeight = 1080;

    qreal mVisibleWidth = 1920;
    qreal mVisibleHeight = 1080;

    bool mRepaintNeeded = false;
    bool mPivotUpdateNeeded = false;

    bool mFirstMouseMove = false;
    bool mSelecting = false;
//    bool mMoving = false;
    QPoint mLastMouseEventPos;
    QPointF mLastPressPos;
    QRectF mSelectionRect;
    CanvasMode mCurrentMode = ADD_POINT;
    MovablePoint *mLastPressedPoint = NULL;
    PathPoint *mCurrentEndPoint = NULL;
    BoundingBox *mLastPressedBox = NULL;
    void setCtrlPointsEnabled(bool enabled);
    PathPivot *mRotPivot;
    void handleMovePointMouseMove(QPointF eventPos);
    void handleMovePathMouseMove(QPointF eventPos);
    void handleAddPointMouseMove(QPointF eventPos);
    void handleMovePathMousePressEvent();
    void handleAddPointMouseRelease();
};

#endif // CANVAS_H
