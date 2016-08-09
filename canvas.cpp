#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include "undoredo.h"
#include "mainwindow.h"

Canvas::Canvas(MainWindow *parent) : QWidget(parent)
{
    mMainWindow = parent;
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
            mSelectionRect.setTopLeft(mPressPos);
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
                mSelectionRect.setTopLeft(mPressPos);
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
}

void Canvas::handleMovePointMouseRelease(QMouseEvent *event) {
    if(mSelecting) {
        mSelectionRect.setBottomRight(event->pos());
        foreach (VectorPath *path, mSelectedPaths) {
            path->SelectAndAddContainedPointsToList(mSelectionRect, &mSelectedPoints);
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

UndoRedoStack *Canvas::getUndoRedoStack()
{
    return mMainWindow->getUndoRedoStack();
}

void Canvas::handleMovePathMouseRelease(QMouseEvent *event) {
    if(mSelecting) {
        mSelectionRect.setBottomRight(event->pos());
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
    repaint();
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(mSelecting) {
        mSelectionRect.setBottomRight(event->pos());
        repaint();
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
        repaint();
    }
    mPressPos = event->pos();
    mFirstMouseMove = false;
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F1) {
        mCurrentMode = CanvasMode::MOVE_PATH;
        repaint();
    } else if(event->key() == Qt::Key_F2) {
        mCurrentMode = CanvasMode::MOVE_POINT;
        repaint();
    } else if(event->key() == Qt::Key_F3) {
        mCurrentMode = CanvasMode::ADD_POINT;
        repaint();
    } else if(event->key() == Qt::Key_Delete) {
        if(mCurrentMode == MOVE_POINT) {
            getUndoRedoStack()->startNewSet();

            foreach(PathPoint *point, mSelectedPoints) {
                point->remove();
            }

            getUndoRedoStack()->finishSet();
        } else if(mCurrentMode == MOVE_PATH) {
            getUndoRedoStack()->startNewSet();

            foreach(VectorPath *path, mSelectedPaths) {
                path->remove();
            }

            getUndoRedoStack()->finishSet();
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
    getUndoRedoStack()->startNewSet();
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

    getUndoRedoStack()->finishSet();
}

void Canvas::addPath(VectorPath *path, bool saveUndoRedo)
{
    mPaths.append(path);
    addPathToSelection(path);

    if(saveUndoRedo) {
        AddPathUndoRedo *undoRedo = new AddPathUndoRedo(path);
        getUndoRedoStack()->addUndoRedo(undoRedo);
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
        getUndoRedoStack()->addUndoRedo(undoRedo);
    }
}
