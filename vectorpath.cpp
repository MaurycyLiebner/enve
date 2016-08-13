#include "vectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "pathpivot.h"

VectorPath::VectorPath(BoxesGroup *group) :
    BoundingBox(group,
                BoundingBoxType::TYPE_VECTOR_PATH)
{
}

VectorPath::~VectorPath()
{
    foreach(PathPoint *point, mPoints) {
        delete point;
    }
}

void VectorPath::updatePivotPosition() {
    if(!mPivotChanged) {
        QPointF posSum = QPointF(0.f, 0.f);
        int count = mPoints.length();
        foreach(PathPoint *point, mPoints) {
            posSum += point->getAbsolutePos();
        }
        mAbsRotPivotPos = posSum/count;
    }
}

PaintSettings VectorPath::getFillSettings()
{
    return mFillSettings;
}

StrokeSettings VectorPath::getStrokeSettings()
{
    return mStrokeSettings;
}

void VectorPath::setFillStrokeSettings(PaintSettings fillSettings,
                                       StrokeSettings strokeSettings)
{
    setFillSettings(fillSettings);
    setStrokeSettings(strokeSettings);
}

void VectorPath::setStrokeSettings(StrokeSettings strokeSettings)
{
    mStrokeSettings = strokeSettings;
    mStrokeSettings.updateQPen();
    updateDrawPen();
    scheduleRepaint();
}

void VectorPath::setFillSettings(PaintSettings fillSettings)
{
    mFillSettings = fillSettings;
    updateDrawGradient();
    scheduleRepaint();
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
            mPath.cubicTo(lastPoint->getEndCtrlPtValue(),
                          point->getStartCtrlPtValue(),
                          pointPos);
            if(point == firstPointInPath) {
                break;
            }
            lastPoint = point;
        }
    }

    updateMappedPath();
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

void VectorPath::updateAfterCombinedTransformationChanged()
{
    scheduleMappedPathUpdate();
}

void VectorPath::updateMappedPath()
{
    mMappedPath = mCombinedTransformMatrix.map(mPath);
    updateDrawPen();
}

void VectorPath::updateDrawPen() {
    mDrawPen = mStrokeSettings.qpen;
    mDrawPen.setWidthF(mDrawPen.widthF()*getCurrentCanvasScale());
}

void VectorPath::updateDrawGradient()
{
    if(mFillSettings.gradient != NULL) {
        mDrawGradient.setStops(mFillSettings.gradient->qgradientStops);
        mDrawGradient.setStart(0.f, 0.f);
        mDrawGradient.setFinalStop(100.f, 100.f);
    }
}

QRectF VectorPath::getBoundingRect()
{
    return mMappedPath.boundingRect();
}

void VectorPath::draw(QPainter *p)
{
    p->save();
    p->setPen(mDrawPen);
    p->setBrush(mFillSettings.color.qcol);
    if(mFillSettings.paintType == GRADIENTPAINT) {
        p->setBrush(mDrawGradient);
    } else if(mFillSettings.paintType == FLATPAINT) {
        p->setBrush(mFillSettings.color.qcol);
    } else{
        p->setBrush(Qt::NoBrush);
    }
    p->drawPath(mMappedPath);
    p->restore();
}

void VectorPath::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    p->save();
    drawBoundingRect(p);
    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        p->setPen(QPen(QColor(0, 0, 0, 125), 2));
        foreach (PathPoint *point, mPoints) {
            point->draw(p, currentCanvasMode);
        }
    } else if(currentCanvasMode == CanvasMode::ADD_POINT) {
        p->setPen(QPen(QColor(0, 0, 0, 125), 2));
        foreach (PathPoint *point, mPoints) {
            if(point->isEndPoint() || point->isSelected()) {
                point->draw(p, currentCanvasMode);
            }
        }
    }
    p->restore();
}

MovablePoint *VectorPath::getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode)
{
    MovablePoint *pointToReturn = NULL;
    foreach (PathPoint *point, mPoints) {
        pointToReturn = point->getPointAtAbsPos(absPtPos, currentCanvasMode);
        if(pointToReturn != NULL) {
            break;
        }
    }
    return pointToReturn;
}

void VectorPath::selectAndAddContainedPointsToList(QRectF absRect,
                                                   QList<MovablePoint *> *list)
{
    foreach(PathPoint *point, mPoints) {
        point->rectPointsSelection(absRect, list);
    }
}

void VectorPath::addPointToSeparatePaths(PathPoint *pointToAdd,
                                         bool saveUndoRedo) {
    mSeparatePaths.append(pointToAdd);
    pointToAdd->setSeparatePathPoint(true);

    if(saveUndoRedo) {
        AddPointToSeparatePathsUndoRedo *undoRedo = new AddPointToSeparatePathsUndoRedo(this, pointToAdd);
        addUndoRedo(undoRedo);
    }
    schedulePathUpdate();
}

void VectorPath::removePointFromSeparatePaths(PathPoint *pointToRemove,
                                              bool saveUndoRedo) {
    pointToRemove->setSeparatePathPoint(false);
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

PathPoint* VectorPath::addPoint(QPointF absPtPos, PathPoint *toPoint)
{
    PathPoint *newPoint = new PathPoint(absPtPos, this);

    return addPoint(newPoint, toPoint);
}

void VectorPath::appendToPointsList(PathPoint *point, bool saveUndoRedo) {
    mPoints.append(point);
    point->show();
    if(saveUndoRedo) {
        AppendToPointsListUndoRedo *undoRedo = new AppendToPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
    }
    updatePivotPosition();
}

void VectorPath::removeFromPointsList(PathPoint *point, bool saveUndoRedo) {
    mPoints.removeOne(point);
    point->hide();
    point->deselect();
    if(saveUndoRedo) {
        RemoveFromPointsListUndoRedo *undoRedo = new RemoveFromPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
    }
    updatePivotPosition();
}

void VectorPath::removePoint(PathPoint *point) {
    startNewUndoRedoSet();
    PathPoint *prevPoint = point->getPreviousPoint();
    PathPoint *nextPoint = point->getNextPoint();

    if(prevPoint != NULL) {
        if(point->isSeparatePathPoint()) {
            replaceSeparatePathPoint(point, prevPoint);
        }
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

void VectorPath::replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                          PathPoint *newPoint) {
    startNewUndoRedoSet();
    removePointFromSeparatePaths(pointBeingReplaced);
    addPointToSeparatePaths(newPoint);
    finishUndoRedoSet();
}
