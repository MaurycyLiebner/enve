#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include "boxesgroup.h"
#include "Colors/color.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>
#include <QThread>
#include "ctrlpoint.h"

class MainWindow;

class UndoRedo;

class UndoRedoStack;

enum CanvasMode : short {
    MOVE_PATH,
    MOVE_POINT,
    ADD_POINT,
    PICK_PATH_SETTINGS
};

class Edge {
public:
    Edge(PathPoint *pt1, PathPoint *pt2, qreal pressedT) {
        mPoint1 = pt1;
        mPoint1EndPt = pt1->getEndCtrlPt();
        mPoint2 = pt2;
        mPoint2StartPt = pt2->getStartCtrlPt();
        mPressedT = pressedT;
    }

    void makePassThrough(QPointF absPos) {
        QPointF p0Pos = mPoint1->getAbsolutePos();
        QPointF p1Pos = mPoint1EndPt->getAbsolutePos();
        QPointF p2Pos = mPoint2StartPt->getAbsolutePos();
        QPointF p3Pos = mPoint2->getAbsolutePos();
        qreal x0 = p0Pos.x();
        qreal y0 = p0Pos.y();
        qreal x1 = p1Pos.x();
        qreal y1 = p1Pos.y();
        qreal x2 = p2Pos.x();
        qreal y2 = p2Pos.y();
        qreal x3 = p3Pos.x();
        qreal y3 = p3Pos.y();

        qreal dx = absPos.x() - calcCubicBezierVal(x0, x1, x2, x3, mPressedT);
        qreal dy = absPos.y() - calcCubicBezierVal(y0, y1, y2, y3, mPressedT);
        while(dx*dx + dy*dy > 1.) {
            x1 += (1. - mPressedT)*dx;
            y1 += (1. - mPressedT)*dy;
            x2 += mPressedT*dx;
            y2 += mPressedT*dy;

            dx = absPos.x() - calcCubicBezierVal(x0, x1, x2, x3, mPressedT);
            dy = absPos.y() - calcCubicBezierVal(y0, y1, y2, y3, mPressedT);
        }


        mPoint1EndPt->setAbsolutePos(QPointF(x1, y1) );
        mPoint2StartPt->setAbsolutePos(QPointF(x2, y2) );
    }

private:
    PathPoint *mPoint1;
    CtrlPoint *mPoint1EndPt;
    PathPoint *mPoint2;
    CtrlPoint *mPoint2StartPt;

    qreal mPressedT;
};

class Canvas : public QWidget, public BoxesGroup
{
    Q_OBJECT
public:
    explicit Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
                    MainWindow *parent = 0);
    ~Canvas();
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

    void updateAfterFrameChanged(int currentFrame);

    void renderCurrentFrameToQImage(QImage *frame);

    QSize getCanvasSize();

    void playPreview();

    void renderCurrentFrameToPreview();

    QMatrix getCombinedRenderTransform();

    void clearPreview();
    QrealKey *getKeyAtPos(qreal relX, qreal relY, qreal y0);
    void getKeysInRect(QRectF selectionRect,
                       QList<QrealKey *> *keysList);

    void centerPivotPosition() {}
    bool processUnfilteredKeyEvent(QKeyEvent *event);

    void fillGradientChanged(Gradient* gradient, bool finish) {
        mCurrentBoxesGroup->setSelectedFillGradient(gradient, finish);
    }

    void strokeGradientChanged(Gradient* gradient, bool finish) {
        mCurrentBoxesGroup->setSelectedStrokeGradient(gradient, finish);
    }

    void fillFlatColorChanged(Color color, bool finish) {
        mCurrentBoxesGroup->setSelectedFillFlatColor(color, finish);
    }

    void strokeFlatColorChanged(Color color, bool finish) {
        mCurrentBoxesGroup->setSelectedStrokeFlatColor(color, finish);
    }

    void fillPaintTypeChanged(PaintType paintType, Color color,
                              Gradient* gradient) {
        mCurrentBoxesGroup->setSelectedFillPaintType(paintType, color,
                                                     gradient);
    }

    void strokePaintTypeChanged(PaintType paintType, Color color,
                                Gradient* gradient) {
        mCurrentBoxesGroup->setSelectedStrokePaintType(paintType, color,
                                                       gradient);
    }

    void strokeCapStyleChanged(Qt::PenCapStyle capStyle) {
        mCurrentBoxesGroup->setSelectedCapStyle(capStyle);
    }

    void strokeJoinStyleChanged(Qt::PenJoinStyle joinStyle) {
        mCurrentBoxesGroup->setSelectedJoinStyle(joinStyle);
    }

    void strokeWidthChanged(qreal strokeWidth, bool finish) {
        mCurrentBoxesGroup->setSelectedStrokeWidth(strokeWidth, finish);
    }

    void startStrokeWidthTransform() {
        mCurrentBoxesGroup->startSelectedStrokeWidthTransform();
    }

    void startStrokeColorTransform() {
        mCurrentBoxesGroup->startSelectedStrokeColorTransform();
    }

    void startFillColorTransform() {
        mCurrentBoxesGroup->startSelectedFillColorTransform();
    }

protected:
//    void updateAfterCombinedTransformationChanged();
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
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

    void nextPreviewFrame();
public slots:
    void connectPointsSlot();
    void disconnectPointsSlot();
    void mergePointsSlot();

    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();
private:
    Edge *mCurrentEdge = NULL;

    bool mPreviewing = false;
    QImage *mCurrentPreviewImg = NULL;
    QTimer *mPreviewFPSTimer = NULL;
    int mCurrentPreviewFrameId;

    bool mCancelTransform = false;
    bool mIsMouseGrabbing = false;

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

    QList<QImage*> mPreviewFrames;
};

#endif // CANVAS_H
