#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"

Canvas::Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
               MainWindow *parent) :
    QWidget(parent),
    BoxesGroup(fillStrokeSettings, parent)
{
    connect(mFillStrokeSettingsWidget, SIGNAL(fillSettingsChanged(PaintSettings, bool)),
            this, SLOT(fillSettingsChanged(PaintSettings, bool)) );
    connect(mFillStrokeSettingsWidget, SIGNAL(strokeSettingsChanged(StrokeSettings, bool)),
            this, SLOT(strokeSettingsChanged(StrokeSettings, bool)) );

    connect(mFillStrokeSettingsWidget, SIGNAL(startFillSettingsTransform()),
            this, SLOT(startFillSettingsTransform()) );
    connect(mFillStrokeSettingsWidget, SIGNAL(startStrokeSettingsTransform()),
            this, SLOT(startStrokeSettingsTransform()) );
    connect(mFillStrokeSettingsWidget, SIGNAL(finishFillSettingsTransform()),
            this, SLOT(finishFillSettingsTransform()) );
    connect(mFillStrokeSettingsWidget, SIGNAL(finishStrokeSettingsTransform()),
            this, SLOT(finishStrokeSettingsTransform()) );

    mCurrentBoxesGroup = this;
    setFocusPolicy(Qt::StrongFocus);
    mRotPivot = new PathPivot(this);
}

QRectF Canvas::getBoundingRect()
{
    QPointF absPos = getAbsolutePos();
    return QRectF(absPos, absPos + QPointF(mVisibleWidth, mVisibleHeight));
}

bool Canvas::processKeyEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_F1) {
        setCanvasMode(CanvasMode::MOVE_PATH);
    } else if(event->key() == Qt::Key_F2) {
        setCanvasMode(CanvasMode::MOVE_POINT);
    } else if(event->key() == Qt::Key_F3) {
        setCanvasMode(CanvasMode::ADD_POINT);
    } else if(event->key() == Qt::Key_Delete) {
        if(mCurrentMode == MOVE_POINT) {
            mCurrentBoxesGroup->removeSelectedPointsAndClearList();
        } else if(mCurrentMode == MOVE_PATH) {
            mCurrentBoxesGroup->removeSelectedBoxesAndClearList();
        }
    } else if(isCtrlPressed() && event->key() == Qt::Key_G) {
        if(isShiftPressed()) {
            mCurrentBoxesGroup->ungroupSelected();
        } else {
            BoxesGroup *newGroup = mCurrentBoxesGroup->groupSelectedBoxes();
            if(newGroup != NULL) {
                setCurrentBoxesGroup(newGroup);
            }
        }
    } else if(event->key() == Qt::Key_PageUp) {
        mCurrentBoxesGroup->moveSelectedBoxesUp();
    } else if(event->key() == Qt::Key_PageDown) {
        mCurrentBoxesGroup->moveSelectedBoxesDown();
    } else if(event->key() == Qt::Key_End) {
        mCurrentBoxesGroup->bringSelectedBoxesToEnd();
    } else if(event->key() == Qt::Key_Home) {
        mCurrentBoxesGroup->bringSelectedBoxesToFront();
    } else if(event->key() == Qt::Key_R && isMovingPath()) {
        setCanvasMode(MOVE_PATH);
        mLastMouseEventPos = mapFromGlobal(QCursor::pos());
        mLastPressPos = mLastMouseEventPos;
        mRotPivot->startRotating();
        mIsMouseGrabbing = true;
        mDoubleClick = false;
        mFirstMouseMove = true;
        setMouseTracking(true);
    } else if(event->key() == Qt::Key_S && isMovingPath()) {
        setCanvasMode(CanvasMode::MOVE_PATH);
        mLastMouseEventPos = mapFromGlobal(QCursor::pos());
        mLastPressPos = mLastMouseEventPos;
        mRotPivot->startScaling();
        mIsMouseGrabbing = true;
        mDoubleClick = false;
        mFirstMouseMove = true;
        setMouseTracking(true);
    } else if(event->key() == Qt::Key_A && isCtrlPressed()) {
        if(isShiftPressed()) {
            mCurrentBoxesGroup->deselectAllBoxes();
        } else {
            mCurrentBoxesGroup->selectAllBoxes();
        }
    } else {
        return false;
    }
    clearAllPointsSelection();

    return true;
}

void Canvas::setCurrentBoxesGroup(BoxesGroup *group) {
    mCurrentBoxesGroup->setIsCurrentGroup(false);
    mCurrentBoxesGroup->clearBoxesSelection();
    mCurrentBoxesGroup->clearPointsSelection();
    mCurrentBoxesGroup = group;
    group->setIsCurrentGroup(true);
}

void Canvas::rotateBoxesBy(qreal rotChange, QPointF absOrigin, bool startTrans)
{
    mCurrentBoxesGroup->rotateSelectedBy(rotChange, absOrigin, startTrans);
}

void Canvas::scaleBoxesBy(qreal scaleBy, QPointF absOrigin, bool startTrans)
{
    mCurrentBoxesGroup->scaleSelectedBy(scaleBy, absOrigin, startTrans);
}

void Canvas::saveToQuery()
{
    foreachBoxInList(mChildren) {
        box->saveToQuery(0);
    }
}

void Canvas::loadAllBoxesFromSql() {
    loadChildrenFromSql("NULL");
}

void Canvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPointF absPos = getAbsolutePos();
    QRectF viewRect = QRectF(absPos,absPos + QPointF(mVisibleWidth, mVisibleHeight));

    p.drawImage(viewRect, mBgImage);

    foreachBoxInList(mChildren){
        box->draw(&p);
    }
    mCurrentBoxesGroup->drawSelected(&p, mCurrentMode);

    p.setPen(QPen(QColor(0, 0, 255, 125), 2.f, Qt::DotLine));
    if(mSelecting) {
        p.drawRect(mSelectionRect);
    }
    if(mCurrentMode == CanvasMode::MOVE_PATH) {
        mRotPivot->draw(&p);
    }

    QPainterPath path;
    path.addRect(0, 0, width() + 1, height() + 1);
    QPainterPath viewRectPath;
    viewRectPath.addRect(viewRect);
    p.setBrush(QColor(0, 0, 0, 125));
    p.setPen(QPen(Qt::black, 2.f));
    p.drawPath(path.subtracted(viewRectPath));

    p.end();
}

bool Canvas::isMovingPath() {
    return mCurrentMode == CanvasMode::MOVE_PATH;
}

qreal Canvas::getCurrentCanvasScale()
{
    return mTransformMatrix.m11();
}

void Canvas::schedulePivotUpdate()
{
    mPivotUpdateNeeded = true;
}

void Canvas::updatePivotIfNeeded()
{
    if(mPivotUpdateNeeded) {
        mPivotUpdateNeeded = false;
        updatePivot();
    }
}

void Canvas::fillSettingsChanged(PaintSettings fillSettings, bool saveUndoRedo)
{
    mCurrentBoxesGroup->setSelectedFillSettings(fillSettings, saveUndoRedo);
    callUpdateSchedulers();
}

void Canvas::strokeSettingsChanged(StrokeSettings strokeSettings, bool saveUndoRedo)
{
    mCurrentBoxesGroup->setSelectedStrokeSettings(strokeSettings, saveUndoRedo);
    callUpdateSchedulers();
}

void Canvas::startStrokeSettingsTransform()
{
    mCurrentBoxesGroup->startSelectedStrokeTransform();
}

void Canvas::startFillSettingsTransform()
{
    mCurrentBoxesGroup->startSelectedFillTransform();
}

void Canvas::finishStrokeSettingsTransform()
{
    mCurrentBoxesGroup->finishSelectedStrokeTransform();
}

void Canvas::finishFillSettingsTransform()
{
    mCurrentBoxesGroup->finishSelectedFillTransform();
}

void Canvas::connectPointsSlot()
{
    mCurrentBoxesGroup->connectPoints();
}

void Canvas::disconnectPointsSlot()
{
    mCurrentBoxesGroup->disconnectPoints();
}

void Canvas::mergePointsSlot()
{
    mCurrentBoxesGroup->mergePoints();
}

void Canvas::makePointCtrlsSymmetric()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
}

void Canvas::makePointCtrlsSmooth()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
}

void Canvas::makePointCtrlsCorner()
{
    mCurrentBoxesGroup->setPointCtrlsMode(CtrlsMode::CTRLS_CORNER);
}

void Canvas::scheduleRepaint()
{
    if(mRepaintNeeded) {
        return;
    }
    mRepaintNeeded = true;
}

void Canvas::repaintIfNeeded()
{
    if(mRepaintNeeded) {
        repaint();
        mRepaintNeeded = false;
    }
}

void Canvas::moveSecondSelectionPoint(QPointF pos) {
    mSelectionRect.setBottomRight(pos);
    scheduleRepaint();
}

void Canvas::startSelectionAtPoint(QPointF pos) {
    mSelectionRect.setTopLeft(pos);
    mSelectionRect.setBottomRight(pos);
    scheduleRepaint();
}

void Canvas::updatePivot() {
    if(mCurrentMode != MOVE_PATH) {
        return;
    }
    if(mCurrentBoxesGroup->isSelectionEmpty() ) {
        mRotPivot->hide();
    } else {
        mRotPivot->show();
        mRotPivot->setAbsolutePos(mCurrentBoxesGroup->getSelectedPivotPos(), false);
    }
}

//void Canvas::updateAfterCombinedTransformationChanged()
//{
//    BoundingBox::updateAfterCombinedTransformationChanged();
//    mRotPivot->updateRotationMappedPath();
//}

void Canvas::setCanvasMode(CanvasMode mode) {
    mCurrentMode = mode;
    if(mCurrentMode == MOVE_PATH) {
        schedulePivotUpdate();
    }
    scheduleRepaint();
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    processKeyEvent(event);
}

void Canvas::clearAllPathsSelection() {
    mCurrentBoxesGroup->clearBoxesSelection();
    if(mLastPressedBox != NULL) {
        mLastPressedBox->deselect();
        mLastPressedBox = NULL;
    }
}

void Canvas::clearAllPointsSelection() {
    mCurrentBoxesGroup->clearPointsSelection();
    if(mLastPressedPoint != NULL) {
        mLastPressedPoint->deselect();
        mLastPressedPoint = NULL;
    }
    setCurrentEndPoint(NULL);
}

void Canvas::setCurrentEndPoint(PathPoint *point)
{
    if(mCurrentEndPoint != NULL) {
        mCurrentEndPoint->deselect();
    }
    if(point != NULL) {
        point->select();
    }
    mCurrentEndPoint = point;
}

void Canvas::selectOnlyLastPressedBox() {
    mCurrentBoxesGroup->clearBoxesSelection();
    if(mLastPressedBox == NULL) {
        return;
    }
    mCurrentBoxesGroup->addBoxToSelection(mLastPressedBox);
}

void Canvas::selectOnlyLastPressedPoint() {
    mCurrentBoxesGroup->clearPointsSelection();
    if(mLastPressedPoint == NULL) {
        return;
    }
    mCurrentBoxesGroup->addPointToSelection(mLastPressedPoint);
}

void getMirroredCtrlPtAbsPos(bool mirror, PathPoint *point,
                             QPointF *startCtrlPtPos, QPointF *endCtrlPtPos) {
    if(mirror) {
        *startCtrlPtPos = point->getEndCtrlPtAbsPos();
        *endCtrlPtPos = point->getStartCtrlPtAbsPos();
    } else {
        *startCtrlPtPos = point->getStartCtrlPtAbsPos();
        *endCtrlPtPos = point->getEndCtrlPtAbsPos();
    }
}

void Canvas::connectPointsFromDifferentPaths(PathPoint *pointSrc,
                                             PathPoint *pointDest) {
    if(pointSrc->getParentPath() == pointDest->getParentPath()) {
        return;
    }
    VectorPath *pathSrc = pointSrc->getParentPath();
    VectorPath *pathDest = pointDest->getParentPath();
    startNewUndoRedoSet();
    setCurrentEndPoint(pointDest);
    if(pointSrc->hasNextPoint()) {
        PathPoint *point = pointSrc;
        bool mirror = pointDest->hasNextPoint();
        while(point != NULL) {
            QPointF startCtrlPtPos;
            QPointF endCtrlPtPos;
            getMirroredCtrlPtAbsPos(mirror, point,
                                    &startCtrlPtPos, &endCtrlPtPos);
            setCurrentEndPoint(mCurrentEndPoint->addPoint(
                                new PathPoint(point->getAbsolutePos(),
                                              startCtrlPtPos,
                                              endCtrlPtPos,
                                              pathDest)) );
            point = point->getNextPoint();
        }
    } else {
        PathPoint *point = pointSrc;
        bool mirror = pointDest->hasPreviousPoint();
        while(point != NULL) {
            QPointF startCtrlPtPos;
            QPointF endCtrlPtPos;
            getMirroredCtrlPtAbsPos(mirror, point,
                                    &startCtrlPtPos, &endCtrlPtPos);
            setCurrentEndPoint(mCurrentEndPoint->addPoint(
                                new PathPoint(point->getAbsolutePos(),
                                              startCtrlPtPos,
                                              endCtrlPtPos,
                                              pathDest)) );
            point = point->getPreviousPoint();
        }
    }
    mCurrentBoxesGroup->removeChild(pathSrc);

    finishUndoRedoSet();
}
