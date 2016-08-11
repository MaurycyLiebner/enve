#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"

bool zLessThan(BoundingBox *box1, BoundingBox *box2)
{
    return box1->getZIndex() > box2->getZIndex();
}

Canvas::Canvas(MainWindow *parent) : QWidget(parent),
    BoundingBox(parent, BoundingBoxType::TYPE_CANVAS)
{
    mRotPivot = new PathPivot(this);
}

void Canvas::callKeyPress(QKeyEvent *event)
{
    keyPressEvent(event);
}

void Canvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);


    foreachBoxInList(mChildren){
        box->draw(&p);
    }
    foreachBoxInList(mSelectedBoxes) {
        box->drawSelected(&p, mCurrentMode);
    }
    p.setPen(QPen(QColor(0, 0, 255, 125), 2.f, Qt::DotLine));
    if(mSelecting) {
        p.drawRect(mSelectionRect);
    }
    if(mCurrentMode == CanvasMode::MOVE_PATH_ROTATE) {
        mRotPivot->draw(&p);
    }

    QPointF absPos = getAbsolutePos();
    p.drawRect(QRectF(absPos, absPos + QPointF(mVisibleWidth, mVisibleHeight)));
    QPainterPath path;
    path.addRect(0, 0, width() + 1, height() + 1);
    QPainterPath viewRectPath;
    viewRectPath.addRect(
                QRectF(absPos,absPos + QPointF(mVisibleWidth, mVisibleHeight)));
    p.setBrush(QColor(0, 0, 0, 125));
    p.setPen(QPen(Qt::black, 0.f));
    p.drawPath(path.subtracted(viewRectPath));
    p.end();
}

bool Canvas::isMovingPath() {
    return mCurrentMode == CanvasMode::MOVE_PATH_ROTATE ||
            mCurrentMode == CanvasMode::MOVE_PATH_SCALE ||
            mCurrentMode == CanvasMode::MOVE_PATH;
}

void Canvas::connectPoints()
{
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    if(selectedPathPoints.count() == 2) {
        startNewUndoRedoSet();

        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        if(firstPoint->isEndPoint() && secondPoint->isEndPoint()) {
            firstPoint->connectToPoint(secondPoint);
        }
        finishUndoRedoSet();
        scheduleRepaint();
    }

    callUpdateSchedulers();
}

void Canvas::disconnectPoints()
{
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    if(selectedPathPoints.count() == 2) {
        startNewUndoRedoSet();

        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        firstPoint->disconnectFromPoint(secondPoint);

        finishUndoRedoSet();
        scheduleRepaint();
    }

    callUpdateSchedulers();
}

void Canvas::mergePoints()
{
    QList<PathPoint*> selectedPathPoints;
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            selectedPathPoints.append( (PathPoint*) point);
        }
    }
    if(selectedPathPoints.count() == 2) {
        startNewUndoRedoSet();

        PathPoint *firstPoint = selectedPathPoints.first();
        PathPoint *secondPoint = selectedPathPoints.last();
        QPointF sumPos = firstPoint->getAbsolutePos() + secondPoint->getAbsolutePos();
        firstPoint->remove();
        secondPoint->moveToAbs(sumPos/2);

        finishUndoRedoSet();
        scheduleRepaint();
    }

    callUpdateSchedulers();
}

void Canvas::setPointCtrlsMode(CtrlsMode mode) {
    startNewUndoRedoSet();
    foreach(MovablePoint *point, mSelectedPoints) {
        if(point->isPathPoint()) {
            ( (PathPoint*)point)->setCtrlsMode(mode);
        }
    }
    finishUndoRedoSet();
    scheduleRepaint();

    callUpdateSchedulers();
}

void Canvas::makePointCtrlsSymmetric()
{
    setPointCtrlsMode(CtrlsMode::CTRLS_SYMMETRIC);
}

void Canvas::makePointCtrlsSmooth()
{
    setPointCtrlsMode(CtrlsMode::CTRLS_SMOOTH);
}

void Canvas::makePointCtrlsCorner()
{
    setPointCtrlsMode(CtrlsMode::CTRLS_CORNER);
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

void Canvas::setCanvasMode(CanvasMode mode) {
    mCurrentMode = mode;
    scheduleRepaint();
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F1) {
        setCanvasMode(CanvasMode::MOVE_PATH);
    } else if(event->key() == Qt::Key_F2) {
        setCanvasMode(CanvasMode::MOVE_POINT);
    } else if(event->key() == Qt::Key_F3) {
        setCanvasMode(CanvasMode::ADD_POINT);
    } else if(event->key() == Qt::Key_Delete) {
        if(mCurrentMode == MOVE_POINT) {
            startNewUndoRedoSet();

            foreach(MovablePoint *point, mSelectedPoints) {
                point->remove();
            }
            mSelectedPoints.clear();

            finishUndoRedoSet();
        } else if(mCurrentMode == MOVE_PATH) {
            startNewUndoRedoSet();

            foreachBoxInList(mSelectedBoxes) {
                removeChild(box);
            }
            mSelectedBoxes.clear();

            finishUndoRedoSet();
        }
    } else if(event->key() == Qt::Key_PageUp) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->moveUp();
        }
    } else if(event->key() == Qt::Key_PageDown) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->moveDown();
        }
    } else if(event->key() == Qt::Key_End) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->bringToEnd();
        }
    } else if(event->key() == Qt::Key_Home) {
        foreach(BoundingBox *box, mSelectedBoxes) {
            box->bringToFront();
        }
    } else if(event->key() == Qt::Key_R && isMovingPath()) {
        setCanvasMode(CanvasMode::MOVE_PATH_ROTATE);
    } else if(event->key() == Qt::Key_S && isMovingPath()) {
        setCanvasMode(CanvasMode::MOVE_PATH_SCALE);
    } else if(event->key() == Qt::Key_G && isMovingPath()) {
        setCanvasMode(CanvasMode::MOVE_PATH);
    } else {
        return;
    }
    clearAllPointsSelection();
}

void Canvas::clearAllPathsSelection() {
    clearBoxesSelection();
    if(mLastPressedBox != NULL) {
        mLastPressedBox->deselect();
        mLastPressedBox = NULL;
    }
}

void Canvas::clearBoxesSelection()
{
    foreachBoxInList(mSelectedBoxes) {
        box->deselect();
    }
    mSelectedBoxes.clear();
}

void Canvas::clearAllPointsSelection() {
    clearPointsSelection();
    if(mLastPressedPoint != NULL) {
        mLastPressedPoint->deselect();
        mLastPressedPoint = NULL;
    }
    setCurrentEndPoint(NULL);
}

void Canvas::clearPointsSelection()
{
    foreach(MovablePoint *point, mSelectedPoints) {
        point->deselect();
    }
    mSelectedPoints.clear();
}

void Canvas::addBoxToSelection(BoundingBox *box) {
    if(box->isSelected()) {
        return;
    }
    box->select();
    mSelectedBoxes.append(box);
    qSort(mSelectedBoxes.begin(), mSelectedBoxes.end(), zLessThan);
}

void Canvas::addPointToSelection(MovablePoint *point)
{
    if(point->isSelected()) {
        return;
    }
    point->select();
    mSelectedPoints.append(point);
}

void Canvas::removeBoxFromSelection(BoundingBox *box) {
    box->deselect();
    mSelectedBoxes.removeOne(box);
}

void Canvas::removePointFromSelection(MovablePoint *point) {
    point->deselect();
    mSelectedPoints.removeOne(point);
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
    clearBoxesSelection();
    if(mLastPressedBox == NULL) {
        return;
    }
    addBoxToSelection(mLastPressedBox);
}

void Canvas::selectOnlyLastPressedPoint() {
    clearPointsSelection();
    if(mLastPressedPoint == NULL) {
        return;
    }
    addPointToSelection(mLastPressedPoint);
}

bool Canvas::isShiftPressed() {
    return QApplication::keyboardModifiers() & Qt::ShiftModifier;
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
    removeChild(pathSrc);

    finishUndoRedoSet();
}

void Canvas::addChild(BoundingBox *box)
{
    BoundingBox::addChild(box);
    addBoxToSelection(box);
}

void Canvas::removeChild(BoundingBox *box)
{
    BoundingBox::removeChild(box);
    if(box->isSelected()) {
        removeBoxFromSelection(box);
    }
}
