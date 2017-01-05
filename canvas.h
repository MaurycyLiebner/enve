#ifndef CANVAS_H
#define CANVAS_H

#include "Boxes/boxesgroup.h"
#include "Colors/color.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>
#include <QThread>
#include "ctrlpoint.h"
#include "Boxes/textbox.h"

class MainWindow;

class UndoRedo;

class UndoRedoStack;

class Circle;

class Rectangle;

enum CanvasMode : short {
    MOVE_PATH,
    MOVE_POINT,
    ADD_POINT,
    ADD_CIRCLE,
    ADD_RECTANGLE,
    ADD_TEXT,
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

    static void getNewRelPosForKnotInsertionAtT(QPointF P0,
                                                QPointF *P1_ptr,
                                                QPointF *P2_ptr,
                                                QPointF P3,
                                                QPointF *new_p_ptr,
                                                QPointF *new_p_start_ptr,
                                                QPointF *new_p_end_ptr,
                                                qreal t) {
        QPointF P1 = *P1_ptr;
        QPointF P2 = *P2_ptr;
        QPointF P0_1 = (1-t)*P0 + t*P1;
        QPointF P1_2 = (1-t)*P1 + t*P2;
        QPointF P2_3 = (1-t)*P2 + t*P3;

        QPointF P01_12 = (1-t)*P0_1 + t*P1_2;
        QPointF P12_23 = (1-t)*P1_2 + t*P2_3;

        QPointF P0112_1223 = (1-t)*P01_12 + t*P12_23;

        *P1_ptr = P0_1;
        *new_p_start_ptr = P01_12;
        *new_p_ptr = P0112_1223;
        *new_p_end_ptr = P12_23;
        *P2_ptr = P2_3;
    }

    static QPointF getRelPosBetweenPointsAtT(qreal t,
                                             QPointF p0Pos,
                                             QPointF p1EndPos,
                                             QPointF p2StartPos,
                                             QPointF p3Pos) {
        qreal x0 = p0Pos.x();
        qreal y0 = p0Pos.y();
        qreal x1 = p1EndPos.x();
        qreal y1 = p1EndPos.y();
        qreal x2 = p2StartPos.x();
        qreal y2 = p2StartPos.y();
        qreal x3 = p3Pos.x();
        qreal y3 = p3Pos.y();

        return QPointF(calcCubicBezierVal(x0, x1, x2, x3, t),
                       calcCubicBezierVal(y0, y1, y2, y3, t) );
    }

    static QPointF getRelPosBetweenPointsAtT(qreal t,
                                             PathPoint *point1,
                                             PathPoint *point2) {
        if(point1 == NULL) return point2->getRelativePos();
        if(point2 == NULL) return point1->getRelativePos();

        CtrlPoint *point1EndPt = point1->getEndCtrlPt();
        CtrlPoint *point2StartPt = point2->getStartCtrlPt();
        QPointF p0Pos = point1->getRelativePos();
        QPointF p1Pos = point1EndPt->getRelativePos();
        QPointF p2Pos = point2StartPt->getRelativePos();
        QPointF p3Pos = point2->getRelativePos();

        return getRelPosBetweenPointsAtT(t, p0Pos, p1Pos, p2Pos, p3Pos);
    }

    void makePassThrough(QPointF absPos) {
        if(!mPoint2->isStartCtrlPtEnabled() ) {
            mPoint2->setStartCtrlPtEnabled(true);
        }
        if(!mPoint1->isEndCtrlPtEnabled() ) {
            mPoint1->setEndCtrlPtEnabled(true);
        }


        QPointF p0Pos = mPoint1->getAbsolutePos();
        QPointF p1Pos = mPoint1EndPt->getAbsolutePos();
        QPointF p2Pos = mPoint2StartPt->getAbsolutePos();
        QPointF p3Pos = mPoint2->getAbsolutePos();

        if(!mEditPath) {
            BoundingBox *parentBox = mPoint1->getParent();
            PathPointValues p1Values = mPoint1->getShapesInfluencedPointValues();
            p0Pos = parentBox->getCombinedTransform().map(
                        p1Values.pointRelPos);
            p1Pos = parentBox->getCombinedTransform().map(
                        p1Values.endRelPos);
            PathPointValues p2Values = mPoint2->getShapesInfluencedPointValues();
            p2Pos = parentBox->getCombinedTransform().map(
                        p2Values.startRelPos);
            p3Pos = parentBox->getCombinedTransform().map(
                        p2Values.pointRelPos);
        }

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


        mPoint1EndPt->moveToAbs(QPointF(x1, y1) );
        mPoint2StartPt->moveToAbs(QPointF(x2, y2) );
    }

    void finishTransform() {
        mPoint1EndPt->finishTransform();
        mPoint2StartPt->finishTransform();
    }

    void startTransform() {
        mPoint1EndPt->startTransform();
        mPoint2StartPt->startTransform();
    }

    void setEditPath(bool bT) {
        mEditPath = bT;
    }

private:
    PathPoint *mPoint1;
    CtrlPoint *mPoint1EndPt;
    PathPoint *mPoint2;
    CtrlPoint *mPoint2StartPt;
    bool mEditPath = true;

    qreal mPressedT;
};

#include "canvaswidget.h"

class Canvas : public BoxesGroup
{
    Q_OBJECT
public:
    explicit Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
                    CanvasWidget *canvasWidget);
    ~Canvas();
    QRectF getPixBoundingRect();
    void addBoxToSelection(BoundingBox *box);
    void clearBoxesSelection();
    void selectOnlyLastPressedBox();
    void removePointFromSelection(MovablePoint *point);
    void removeBoxFromSelection(BoundingBox *box);
    void selectOnlyLastPressedPoint();
    void connectPointsFromDifferentPaths(PathPoint *pointSrc, PathPoint *pointDest);

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

    void awaitUpdate() {}

    void saveToSql(QSqlQuery *query);
    void loadAllBoxesFromSql(bool loadInBox);
    void clearAll();
    void resetTransormation();
    void fitCanvasToSize();
    void saveSelectedToSqlForCurrentBox(QSqlQuery *query);
    bool processFilteredKeyEvent(QKeyEvent *event);
    void scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin);
    void scale(qreal scaleBy, QPointF absOrigin);
    void pickPathForSettings();
    void moveBy(QPointF trans);

    void updateAfterFrameChanged(int currentFrame);

    void renderCurrentFrameToQImage(QImage *frame);
    void renderFinalCurrentFrameToQImage(QImage *frame);

    QSize getCanvasSize();

    void playPreview();

    void renderCurrentFrameToPreview();

    QMatrix getCombinedRenderTransform();

    void clearPreview();

    void centerPivotPosition(bool finish = false) { Q_UNUSED(finish); }
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

    void updateDisplayedFillStrokeSettings();
    void scaleBoxesBy(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin, bool startTrans);
    void updateInputValue();
    void clearAndDisableInput();

    qreal getCurrentCanvasScale();

    void ctrlsVisiblityChanged();
    void grabMouseAndTrack();

    void setPartialRepaintRect(QRectF absRect);
    void makePartialRepaintInclude(QPointF pointToInclude);
    void partialRepaintRectToPoint(QPointF point);
    void enableHighQualityPaint();
    void disableHighQualityPaint();
    bool highQualityPaint();

    static void enableEffectsPaint();
    static void disableEffectsPaint();
    static bool effectsPaintEnabled();

    static qreal getResolutionPercent();
    static void setResolutionPercent(qreal percent);

    void updateRenderRect();
    QMatrix getCombinedFinalRenderTransform();
    void renderCurrentFrameToOutput(QString renderDest);
    void drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY);
    void showContextMenu(QPoint globalPos);
    void updateAllBoxes();
    void scalePointsBy(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin, bool startTrans);
    void rotatePointsBy(qreal rotChange, QPointF absOrigin, bool startTrans);

    void paintEvent(QPainter *p);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void render(QPainter *p);
    void renderFinal(QPainter *p);
protected:
//    void updateAfterCombinedTransformationChanged();

    void addPointToSelection(MovablePoint *point);

    void setCurrentEndPoint(PathPoint *point);

    PathPoint *getCurrentPoint();

    void handleMovePathMouseRelease(QPointF pos);
    void handleMovePointMouseRelease(QPointF pos);

    bool isMovingPath();
    bool handleKeyPressEventWhileMouseGrabbing(QKeyEvent *event);
    void handleRightButtonMousePress(QMouseEvent *event);
    void handleLeftButtonMousePress(QMouseEvent *event);
signals:
private slots:

    void nextPreviewFrame();
public slots:
    void raiseAction();
    void lowerAction();
    void raiseToTopAction();
    void lowerToBottomAction();

    void objectsToPathAction();

    void setFontFamilyAndStyle(QString family, QString style);
    void setFontSize(qreal size);

    void setMovePathMode() {
        setCanvasMode(MOVE_PATH);

        callUpdateSchedulers();
    }

    void setMovePointMode() {
        setCanvasMode(MOVE_POINT);

        callUpdateSchedulers();
    }

    void setAddPointMode() {
        setCanvasMode(ADD_POINT);

        callUpdateSchedulers();
    }

    void setRectangleMode() {
        setCanvasMode(ADD_RECTANGLE);
    }

    void setCircleMode() {
        setCanvasMode(ADD_CIRCLE);
    }

    void setTextMode() {
        setCanvasMode(ADD_TEXT);
    }

    void connectPointsSlot();
    void disconnectPointsSlot();
    void mergePointsSlot();

    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();
private:
    bool mPivotVisibleDuringPointEdit = true;
    static bool mEffectsPaintEnabled;
    static qreal mResolutionPercent;

    CanvasWidget *mCanvasWidget;

    QRectF mRenderRect;

    Circle *mCurrentCircle = NULL;
    Rectangle *mCurrentRectangle = NULL;
    TextBox *mCurrentTextBox = NULL;

    bool mTransformationFinishedBeforeMouseRelease = false;
    QString mInputText;
    qreal mInputTransformationValue = 0.;
    bool mInputTransformationEnabled = false;

    bool mXOnlyTransform = false;
    bool mYOnlyTransform = false;

    Edge *mCurrentEdge = NULL;

    bool mPreviewing = false;
    QImage *mCurrentPreviewImg = NULL;
    QTimer *mPreviewFPSTimer = NULL;
    int mCurrentPreviewFrameId;

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
    void updateTransformation();
    void handleMouseRelease(QPointF eventPos);
    QPointF getMoveByValueForEventPos(QPointF eventPos);
    void cancelCurrentTransform();
    void releaseMouseAndDontTrack();
    void groupSelectedBoxesAction();
};

#endif // CANVAS_H
