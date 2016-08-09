#include "vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"

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
    mPathUpdateNeeded = true;
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
    mMappedPathUpdateNeeded = true;
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
    repaint();
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
}

void VectorPath::deselect()
{
    mSelected = false;
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

void VectorPath::SelectAndAddContainedPointsToList(QRectF absRect,
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
        getUndoRedoStack()->addUndoRedo(undoRedo);
    }
}

void VectorPath::removePointFromSeparatePaths(PathPoint *pointToRemove,
                                              bool saveUndoRedo) {
    mSeparatePaths.removeOne(pointToRemove);

    if(saveUndoRedo) {
        RemovePointFromSeparatePathsUndoRedo *undoRedo = new RemovePointFromSeparatePathsUndoRedo(this, pointToRemove);
        getUndoRedoStack()->addUndoRedo(undoRedo);
    }

}

PathPoint *VectorPath::addPoint(PathPoint *pointToAdd, PathPoint *toPoint)
{
    UndoRedoStack *stack = getUndoRedoStack();
    stack->startNewSet();
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
    stack->finishSet();

    repaint();

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
        getUndoRedoStack()->addUndoRedo(undoRedo);
    }
    repaint();
}

void VectorPath::removeFromPointsList(PathPoint *point, bool saveUndoRedo) {
    mPoints.removeOne(point);

    if(saveUndoRedo) {
        RemoveFromPointsListUndoRedo *undoRedo = new RemoveFromPointsListUndoRedo(point, this);
        getUndoRedoStack()->addUndoRedo(undoRedo);
    }
    repaint();
}

void VectorPath::removePoint(PathPoint *point) {
    UndoRedoStack *stack = getUndoRedoStack();
    stack->startNewSet();
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

    stack->finishSet();
}

bool VectorPath::pointInsidePath(QPointF point)
{
    return mMappedPath.contains(point);
}

void VectorPath::repaint() {
    getCanvas()->repaint();
}

void VectorPath::replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                          PathPoint *newPoint) {
    mSeparatePaths.replace(mSeparatePaths.indexOf(pointBeingReplaced),
                           newPoint);
}
