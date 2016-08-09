#include "vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"

VectorPath::VectorPath(Canvas *canvas) : ChildParent(canvas)
{
}

VectorPath::~VectorPath()
{
    foreach(PathPoint *point, mPoints) {
        delete point;
    }
}

void VectorPath::updatePath()
{
    mPath = QPainterPath();
    mPath.setFillRule(Qt::WindingFill);
    foreach (PathPoint *firstPointInPath, mSeparatePaths) {
        PathPoint *point = NULL;
        PathPoint *lastPoint = firstPointInPath;
        mPath.moveTo(firstPointInPath->getRelativePos());
        while(true) {
            point = lastPoint->getNextPoint();
            if(point == NULL) {
                break;
            }
            QPointF pointPos = point->getRelativePos();
            mPath.cubicTo(pointPos + point->getEndCtrlPt(),
                          pointPos + point->getStartCtrlPt(),
                          pointPos);
            if(point == firstPointInPath) {
                break;
            }
            lastPoint = point;
        }
    }

    updateMappedPath();
}

void VectorPath::remove() {
    mCanvas->removePath(this);
}

void VectorPath::schedulePathUpdate()
{
    if(mPathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new PathUpdateScheduler(this));
    mPathUpdateNeeded = true;
    mMappedPathUpdateNeeded = false;
    scheduleRepaint();
}

void VectorPath::updatePathIfNeeded()
{
    if(mPathUpdateNeeded) {
        mPathUpdateNeeded = false;
        mMappedPathUpdateNeeded = false;
        updatePath();
    }
}

void VectorPath::updateMappedPathIfNeeded()
{
    if(mMappedPathUpdateNeeded) {
        mMappedPathUpdateNeeded = false;
        updateMappedPath();
    }
}

void VectorPath::scheduleMappedPathUpdate()
{
    if(mMappedPathUpdateNeeded || mPathUpdateNeeded) {
        return;
    }
    addUpdateScheduler(new MappedPathUpdateScheduler(this));
    mMappedPathUpdateNeeded = true;
    scheduleRepaint();
}

void VectorPath::updateAfterTransformationChanged()
{
    return;
}

void VectorPath::updateAfterCombinedTransformationChanged()
{
    scheduleMappedPathUpdate();
}

void VectorPath::updateMappedPath()
{
    mMappedPath = mCombinedTransformMatrix.map(mPath);
}

bool VectorPath::isContainedIn(QRectF absRect) {
    return absRect.contains(getBoundingRect());
}

bool VectorPath::isSelected()
{
    return mSelected;
}

void VectorPath::select()
{
    mSelected = true;
    scheduleRepaint();
}

void VectorPath::deselect()
{
    mSelected = false;
    scheduleRepaint();
}

QRectF VectorPath::getBoundingRect()
{
    return mMappedPath.boundingRect();
}

void VectorPath::draw(QPainter *p)
{
    p->save();
    p->setBrush(QColor(200, 200, 200));
    p->drawPath(mMappedPath);
    p->restore();
}

void VectorPath::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    p->save();
    if(currentCanvasMode == CanvasMode::MOVE_PATH) {
        p->setPen(QPen(QColor(0, 0, 0, 125), 2, Qt::DotLine));
        p->setBrush(Qt::NoBrush);
        p->drawRect(getBoundingRect());
    } else if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        p->setPen(QPen(QColor(0, 0, 0, 125), 2));
        foreach (PathPoint *point, mPoints) {
            point->draw(p);
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        p->setPen(QPen(QColor(0, 0, 0, 125), 2));
        foreach (PathPoint *point, mPoints) {
            if(point->isEndPoint()) {
                point->draw(p);
            }
        }
    }
    p->restore();
}

PathPoint *VectorPath::getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode)
{
    foreach (PathPoint *point, mPoints) {
        if(point->isPointAt(absPtPos)) {
            if(currentCanvasMode == CanvasMode::MOVE_POINT) {
                return point;
            } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
                if(point->isEndPoint()) {
                    return point;
                }
            }
        }
    }
    return NULL;
}

void VectorPath::selectAndAddContainedPointsToList(QRectF absRect,
                                                   QList<PathPoint *> *list)
{
    foreach(PathPoint *point, mPoints) {
        if(point->isSelected()) {
            continue;
        }
        if(point->isContainedInRect(absRect)) {
            point->select();
            list->append(point);
        }
    }
}

void VectorPath::addPointToSeparatePaths(PathPoint *pointToAdd,
                                         bool saveUndoRedo) {
    mSeparatePaths.append(pointToAdd);

    if(saveUndoRedo) {
        AddPointToSeparatePathsUndoRedo *undoRedo = new AddPointToSeparatePathsUndoRedo(this, pointToAdd);
        addUndoRedo(undoRedo);
    }

    schedulePathUpdate();
}

void VectorPath::removePointFromSeparatePaths(PathPoint *pointToRemove,
                                              bool saveUndoRedo) {
    mSeparatePaths.removeOne(pointToRemove);

    if(saveUndoRedo) {
        RemovePointFromSeparatePathsUndoRedo *undoRedo = new RemovePointFromSeparatePathsUndoRedo(this, pointToRemove);
        addUndoRedo(undoRedo);
    }
    schedulePathUpdate();
}

PathPoint *VectorPath::addPoint(PathPoint *pointToAdd, PathPoint *toPoint)
{
    startNewUndoRedoSet();
    appendToPointsList(pointToAdd);
    if(toPoint == NULL) {
        addPointToSeparatePaths(pointToAdd);
    } else {
        if(!toPoint->hasNextPoint()) {
            toPoint->setPointAsNext(pointToAdd);
        } else if(!toPoint->hasPreviousPoint() ) {
            replaceSeparatePathPoint(toPoint, pointToAdd);
            toPoint->setPointAsPrevious(pointToAdd);
        }
    }
    finishUndoRedoSet();

    return pointToAdd;
}

Canvas *VectorPath::getCanvas()
{
    return mCanvas;
}

PathPoint* VectorPath::addPoint(QPointF absPtPos, PathPoint *toPoint)
{
    PathPoint *newPoint = new PathPoint(absPtPos, this);

    return addPoint(newPoint, toPoint);
}

void VectorPath::appendToPointsList(PathPoint *point, bool saveUndoRedo) {
    mPoints.append(point);

    if(saveUndoRedo) {
        AppendToPointsListUndoRedo *undoRedo = new AppendToPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
    }
}

void VectorPath::removeFromPointsList(PathPoint *point, bool saveUndoRedo) {
    mPoints.removeOne(point);

    if(saveUndoRedo) {
        RemoveFromPointsListUndoRedo *undoRedo = new RemoveFromPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
    }
}

void VectorPath::removePoint(PathPoint *point) {
    startNewUndoRedoSet();
    PathPoint *prevPoint = point->getPreviousPoint();
    PathPoint *nextPoint = point->getNextPoint();

    if(prevPoint != NULL) {
        prevPoint->setPointAsNext(nextPoint);
    } else if (nextPoint != NULL){
        replaceSeparatePathPoint(point, nextPoint);
        nextPoint->setPointAsPrevious(NULL);
    } else {
        removePointFromSeparatePaths(point);
    }
    removeFromPointsList(point);

    finishUndoRedoSet();
}

bool VectorPath::pointInsidePath(QPointF point)
{
    return mMappedPath.contains(point);
}

void VectorPath::scheduleRepaint() {
    getCanvas()->scheduleRepaint();
}

void VectorPath::replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                          PathPoint *newPoint) {
    mSeparatePaths.replace(mSeparatePaths.indexOf(pointBeingReplaced),
                           newPoint);
    schedulePathUpdate();
}
