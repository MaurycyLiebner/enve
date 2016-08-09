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

void Canvas::mousePressEvent(QMouseEvent *event)
{
    mFirstMouseMove = true;
    mPressPos = event->pos();
    if(mCurrentMode == CanvasMode::MOVE_PATH) {
        VectorPath *pathUnderMouse = NULL;
        foreach(VectorPath *path, mPaths) {
            if(path->pointInsidePath(mPressPos)) {
                if(!(isShiftPressed()) && !path->isSelected()) {
                    clearPathsSelection();
                }
                pathUnderMouse = path;
                break;
            }
        }
        if(pathUnderMouse == NULL) {
            if(!(isShiftPressed()) ) {
                clearPathsSelection();
            }
            mSelecting = true;
            startSelectionAtPoint(mPressPos);
        }
        mLastPressedPath = pathUnderMouse;
    } else {
        PathPoint *pointUnderMouse = NULL;
        foreach (VectorPath *path, mSelectedPaths) {
            pointUnderMouse = path->getPointAt(mPressPos, mCurrentMode);
            if(pointUnderMouse != NULL) {
                break;
            }
        }
        mLastPressedPoint = pointUnderMouse;


        if(mCurrentMode == CanvasMode::ADD_POINT) {
            if(pointUnderMouse == mCurrentPoint && pointUnderMouse != NULL) {
                return;
            }
            if(mCurrentPoint == NULL && pointUnderMouse == NULL) {
                VectorPath *newPath = new VectorPath(this);
                addPath(newPath);
                setCurrentPoint(newPath->addPoint(mPressPos, mCurrentPoint) );
                newPath->updatePathIfNeeded();
            } else {
                if(pointUnderMouse == NULL) {
                    setCurrentPoint(mCurrentPoint->addPoint(mPressPos) );
                    mCurrentPoint->getParentPath()->updatePathIfNeeded();
                } else if(mCurrentPoint == NULL) {
                        setCurrentPoint(pointUnderMouse);
                } else {
                    if(mCurrentPoint->getParentPath() == pointUnderMouse->getParentPath())
                    {
                        pointUnderMouse->connectToPoint(mCurrentPoint);
                        pointUnderMouse->getParentPath()->updatePathIfNeeded();
                    }
                    else {
                        connectPointsFromDifferentPaths(pointUnderMouse, mCurrentPoint);
                        mCurrentPoint->getParentPath()->updatePathIfNeeded();
                    }
                    setCurrentPoint(NULL);
                }
            } // pats is not null
        } // point adding mode
        else if (mCurrentMode == CanvasMode::MOVE_POINT) {
            if (pointUnderMouse == NULL) {
                if(!(isShiftPressed()) ) {
                    clearPointsSelection();
                }
                mSelecting = true;
                startSelectionAtPoint(mPressPos);
            } else {
                if(pointUnderMouse->isSelected()) {
                    return;
                }
                if(!isShiftPressed()) {
                    clearPointsSelection();
                }
            }
        }
    } // current mode allows interaction with points

    callUpdateSchedulers();
}

void Canvas::handleMovePointMouseRelease(QMouseEvent *event) {
    if(mSelecting) {
        moveSecondSelectionPoint(event->pos());
        foreach (VectorPath *path, mSelectedPaths) {
            path->selectAndAddContainedPointsToList(mSelectionRect, &mSelectedPoints);
        }
        mSelecting = false;
    } else if(mFirstMouseMove) {
        if(isShiftPressed()) {
            if(mLastPressedPoint != NULL) {
                if(mLastPressedPoint->isSelected()) {
                    removePointFromSelection(mLastPressedPoint);
                } else {
                    addPointToSelection(mLastPressedPoint);
                }
            }
        } else {
            selectOnlyLastPressedPoint();
        }
    } else {
        foreach(PathPoint *point, mSelectedPoints) {
            point->finishTransform();
        }
    }
}

void Canvas::scheduleRepaint()
{
    if(mRepaintNeeded) {
        return;
    }
    addUpdateScheduler(new CanvasRepaintScheduler(this));
    mRepaintNeeded = true;
}

void Canvas::repaintIfNeeded()
{
    if(mRepaintNeeded) {
        repaint();
        mRepaintNeeded = false;
    }
}

void Canvas::handleMovePathMouseRelease(QMouseEvent *event) {
    if(mSelecting) {
        moveSecondSelectionPoint(event->pos());
        foreach (VectorPath *path, mPaths) {
            if(path->isContainedIn(mSelectionRect) ) {
                addPathToSelection(path);
            }
        }
        mSelecting = false;
    } else if(mFirstMouseMove) {
        if(isShiftPressed()) {
            if(mLastPressedPath != NULL) {
                if(mLastPressedPath->isSelected()) {
                    removePathFromSelection(mLastPressedPath);
                } else {
                    addPathToSelection(mLastPressedPath);
                }
            }
        } else {
            selectOnlyLastPressedPath();
        }
    } else {
        foreach(VectorPath *path, mSelectedPaths) {
            path->finishTransform();
        }
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(mCurrentMode == CanvasMode::MOVE_POINT) {
        handleMovePointMouseRelease(event);
    } else if(mCurrentMode == CanvasMode::MOVE_PATH) {
        handleMovePathMouseRelease(event);
    }
    mLastPressedPath = NULL;
    mLastPressedPoint = NULL;

    callUpdateSchedulers();
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

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(mSelecting) {
        moveSecondSelectionPoint(event->pos());
    } else if(mCurrentMode == CanvasMode::MOVE_POINT) {
        if(mLastPressedPoint != NULL) {
            addPointToSelection(mLastPressedPoint);
            mLastPressedPoint = NULL;
        }
        if(mFirstMouseMove) {
            foreach(PathPoint *point, mSelectedPoints) {
                point->startTransform();
                point->moveBy(event->pos() - mPressPos);
                point->getParentPath()->updatePathIfNeeded();
            }
        } else {
            foreach(PathPoint *point, mSelectedPoints) {
                point->moveBy(event->pos() - mPressPos);
                point->getParentPath()->updatePathIfNeeded();
            }
        }
    } else if(mCurrentMode == CanvasMode::MOVE_PATH) {
        if(mLastPressedPath != NULL) {
            addPathToSelection(mLastPressedPath);
            mLastPressedPath = NULL;
        }
        if(mFirstMouseMove) {
            foreach(VectorPath *path, mSelectedPaths) {
                path->startTransform();
                path->moveBy(event->pos() - mPressPos);
                path->updateMappedPathIfNeeded();
            }
        } else {
            foreach(VectorPath *path, mSelectedPaths) {
                path->moveBy(event->pos() - mPressPos);
                path->updateMappedPathIfNeeded();
            }
        }
    }
    mPressPos = event->pos();
    mFirstMouseMove = false;

    callUpdateSchedulers();
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

            foreach(PathPoint *point, mSelectedPoints) {
                point->remove();
            }

            finishUndoRedoSet();
        } else if(mCurrentMode == MOVE_PATH) {
            startNewUndoRedoSet();

            foreach(VectorPath *path, mSelectedPaths) {
                path->remove();
            }

            finishUndoRedoSet();
        }
    } else {
        return;
    }
    setCurrentPoint(NULL);
    clearPointsSelection();
}

void Canvas::clearPathsSelection()
{
    foreach(VectorPath *path, mSelectedPaths) {
        path->deselect();
    }
    mSelectedPaths.clear();
}

void Canvas::clearPointsSelection()
{
    foreach(PathPoint *point, mSelectedPoints) {
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

void Canvas::addPointToSelection(PathPoint *point)
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

void Canvas::removePointFromSelection(PathPoint *point) {
    point->deselect();
    mSelectedPoints.removeOne(point);
}

void Canvas::setCurrentPoint(PathPoint *point)
{
    if(mCurrentPoint != NULL) {
        mCurrentPoint->deselect();
    }
    if(point != NULL) {
        point->select();
    }
    mCurrentPoint = point;
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


void Canvas::connectPointsFromDifferentPaths(PathPoint *pointSrc,
                                             PathPoint *pointDest) {
    if(pointSrc->getParentPath() == pointDest->getParentPath()) {
        return;
    }
    startNewUndoRedoSet();
    setCurrentPoint(pointDest);
    if(pointSrc->hasNextPoint()) {
        PathPoint *point = pointSrc;
        while(point != NULL) {
            setCurrentPoint(mCurrentPoint->addPoint(point->getAbsolutePos()));
            point = point->getNextPoint();
        }
    } else {
        PathPoint *point = pointSrc;
        while(point != NULL) {
            setCurrentPoint(mCurrentPoint->addPoint(point->getAbsolutePos()));
            point = point->getPreviousPoint();
        }
    }
    VectorPath *pathSrc = pointSrc->getParentPath();
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
