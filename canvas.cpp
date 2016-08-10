#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"

Canvas::Canvas(MainWindow *parent) : QWidget(parent), ConnectedToMainWindow(parent)
{
}

void Canvas::callKeyPress(QKeyEvent *event)
{
    keyPressEvent(event);
}

void Canvas::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    foreach (VectorPath *path, mPaths) {
        path->draw(&p);
    }
    foreach (VectorPath *path, mSelectedPaths) {
        path->drawSelected(&p, mCurrentMode);
    }
    p.setPen(QPen(QColor(0, 0, 255, 125), 2.f, Qt::DotLine));
    if(mSelecting) {
        p.drawRect(mSelectionRect);
    }
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

            foreach(VectorPath *path, mSelectedPaths) {
                path->remove();
            }
            mSelectedPaths.clear();

            finishUndoRedoSet();
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
    clearPathsSelection();
    if(mLastPressedPath != NULL) {
        mLastPressedPath->deselect();
        mLastPressedPath = NULL;
    }
}

void Canvas::clearPathsSelection()
{
    foreach(VectorPath *path, mSelectedPaths) {
        path->deselect();
    }
    mSelectedPaths.clear();
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

void Canvas::addPathToSelection(VectorPath *path) {
    if(path->isSelected()) {
        return;
    }
    path->select();
    mSelectedPaths.append(path);
}

void Canvas::addPointToSelection(MovablePoint *point)
{
    if(point->isSelected()) {
        return;
    }
    point->select();
    mSelectedPoints.append(point);
}

void Canvas::removePathFromSelection(VectorPath *path) {
    path->deselect();
    mSelectedPaths.removeOne(path);
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

void Canvas::selectOnlyLastPressedPath() {
    clearPathsSelection();
    if(mLastPressedPath == NULL) {
        return;
    }
    addPathToSelection(mLastPressedPath);
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
    removePath(pathSrc);

    finishUndoRedoSet();
}

void Canvas::addPath(VectorPath *path, bool saveUndoRedo)
{
    mPaths.append(path);
    addPathToSelection(path);

    if(saveUndoRedo) {
        AddPathUndoRedo *undoRedo = new AddPathUndoRedo(path);
        addUndoRedo(undoRedo);
    }
}

void Canvas::removePath(VectorPath *path, bool saveUndoRedo)
{
    mPaths.removeOne(path);
    if(path->isSelected()) {
        removePathFromSelection(path);
    }

    if(saveUndoRedo) {
        RemovePathUndoRedo *undoRedo = new RemovePathUndoRedo(path);
        addUndoRedo(undoRedo);
    }
}
