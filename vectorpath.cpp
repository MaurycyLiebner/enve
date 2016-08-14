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
    mFillGradientPoints.initialize(this);
    mStrokeGradientPoints.initialize(this);
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
    return mFillPaintSettings;
}

StrokeSettings VectorPath::getStrokeSettings()
{
    return mStrokeSettings;
}

void VectorPath::setStrokeSettings(StrokeSettings strokeSettings, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new StrokeSettingsChangedUndoRedo(mStrokeSettings,
                                                      strokeSettings,
                                                      this) );
    }
    bool wasGradient = mStrokeSettings.paintType == GRADIENTPAINT;
    if(wasGradient) {
        mStrokeSettings.gradient->removePath(this);
    }
    mStrokeSettings = strokeSettings;
    updateOutlinePath();
    if(mStrokeSettings.paintType == GRADIENTPAINT && !wasGradient) {
        mStrokeGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), saveUndoRedo);
    }
    scheduleRepaint();
}

void VectorPath::setFillSettings(PaintSettings fillSettings, bool saveUndoRedo)
{
    if(saveUndoRedo) {
        addUndoRedo(new FillSettingsChangedUndoRedo(mFillPaintSettings,
                                                       fillSettings,
                                                     this) );
    }
    bool wasGradient = mFillPaintSettings.paintType == GRADIENTPAINT;
    if(wasGradient) {
        mFillPaintSettings.gradient->removePath(this);
    }
    mFillPaintSettings = fillSettings;
    updateDrawGradients();
    if(mFillPaintSettings.paintType == GRADIENTPAINT && !wasGradient) {
        mFillGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), saveUndoRedo);
    }
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

void VectorPath::updateOutlinePath() {
    mStrokeSettings.setStrokerSettings(&mPathStroker, getCurrentCanvasScale());
    mOutlinePath = mPathStroker.createStroke(mMappedPath);
    updateWholePath();
}

void VectorPath::updateWholePath() {
    mMappedWhole = QPainterPath();
    if(mStrokeSettings.paintType != NOPAINT) {
        mMappedWhole += mOutlinePath;
    }
    if(mFillPaintSettings.paintType != NOPAINT ||
            mStrokeSettings.paintType == NOPAINT) {
        mMappedWhole += mMappedPath;
    }
}

void VectorPath::updateMappedPath()
{
    mMappedPath = mCombinedTransformMatrix.map(mPath);
    updateOutlinePath();
    updateDrawGradients();
}

void VectorPath::updateDrawGradients()
{
    if(mFillPaintSettings.paintType == GRADIENTPAINT) {
        Gradient *gradient = mFillPaintSettings.gradient;
        if(!gradient->isInPaths(this)) {
            gradient->addPath(this);
        }
        mFillGradientPoints.setColors(gradient->qgradientStops.first().second,
                                      gradient->qgradientStops.last().second);
        mFillGradientPoints.enable();
        mDrawFillGradient.setStops(gradient->qgradientStops);
        mDrawFillGradient.setStart(mFillGradientPoints.getStartPoint() );
        mDrawFillGradient.setFinalStop(mFillGradientPoints.getEndPoint() );
    } else {
        mFillGradientPoints.disable();
    }
    if(mStrokeSettings.paintType == GRADIENTPAINT) {
        Gradient *gradient = mStrokeSettings.gradient;
        if(!gradient->isInPaths(this)) {
            gradient->addPath(this);
        }
        mStrokeGradientPoints.setColors(gradient->qgradientStops.first().second,
                                      gradient->qgradientStops.last().second);

        mStrokeGradientPoints.enable();
        mDrawStrokeGradient.setStops(gradient->qgradientStops);
        mDrawStrokeGradient.setStart(mStrokeGradientPoints.getStartPoint() );
        mDrawStrokeGradient.setFinalStop(mStrokeGradientPoints.getEndPoint() );
    } else {
        mStrokeGradientPoints.disable();
    }
}

void VectorPath::startStrokeTransform()
{
    mSavedStrokeSettings = mStrokeSettings;
}

void VectorPath::startFillTransform()
{
    mSavedFillPaintSettings = mFillPaintSettings;
}

void VectorPath::finishStrokeTransform()
{
    addUndoRedo(new StrokeSettingsChangedUndoRedo(mSavedStrokeSettings,
                                                  mStrokeSettings,
                                                  this) );
}

void VectorPath::finishFillTransform()
{
    addUndoRedo(new FillSettingsChangedUndoRedo(mSavedFillPaintSettings,
                                                mFillPaintSettings,
                                                this) );
}

QRectF VectorPath::getBoundingRect()
{
    return mMappedWhole.boundingRect();
}

void VectorPath::draw(QPainter *p)
{
    p->save();
    p->setPen(Qt::NoPen);
    if(mFillPaintSettings.paintType == GRADIENTPAINT) {
        p->setBrush(mDrawFillGradient);
    } else if(mFillPaintSettings.paintType == FLATPAINT) {
        p->setBrush(mFillPaintSettings.color.qcol);
    } else{
        p->setBrush(Qt::NoBrush);
    }
    p->drawPath(mMappedPath);
    if(mStrokeSettings.paintType == GRADIENTPAINT) {
        p->setBrush(mDrawStrokeGradient);
    } else if(mStrokeSettings.paintType == FLATPAINT) {
        p->setBrush(mStrokeSettings.color.qcol);
    } else{
        p->setBrush(Qt::NoBrush);
    }
    p->drawPath(mOutlinePath);
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
        mFillGradientPoints.draw(p);
        mStrokeGradientPoints.draw(p);
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
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints.getPointAt(absPtPos);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints.getPointAt(absPtPos);
        }
    }
    if(pointToReturn == NULL) {
        foreach (PathPoint *point, mPoints) {
            pointToReturn = point->getPointAtAbsPos(absPtPos, currentCanvasMode);
            if(pointToReturn != NULL) {
                break;
            }
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
        startNewUndoRedoSet();
        RemoveFromPointsListUndoRedo *undoRedo = new RemoveFromPointsListUndoRedo(point, this);
        addUndoRedo(undoRedo);
        if(mPoints.count() < 2) {
            mParent->removeChild(this);
        }
        finishUndoRedoSet();
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
    return mMappedWhole.contains(point);
}

void VectorPath::replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                          PathPoint *newPoint) {
    startNewUndoRedoSet();
    removePointFromSeparatePaths(pointBeingReplaced);
    addPointToSeparatePaths(newPoint);
    finishUndoRedoSet();
}

GradientPoints::GradientPoints()
{

}

void GradientPoints::initialize(VectorPath *parentT)
{
    parent = parentT;
    startPoint = new GradientPoint(QPointF(0, 0), parent);
    endPoint = new GradientPoint(QPointF(100, 100), parent);
    enabled = false;
}

void GradientPoints::enable()
{
    if(enabled) {
        return;
    }
    enabled = true;
}

void GradientPoints::setPositions(QPointF startPos, QPointF endPos, bool saveUndoRedo) {
    startPoint->setAbsolutePos(startPos, saveUndoRedo);
    endPoint->setAbsolutePos(endPos, saveUndoRedo);
}

void GradientPoints::disable()
{
    enabled = false;
}

void GradientPoints::draw(QPainter *p)
{
    if(enabled) {
       p->drawLine(startPoint->getAbsolutePos(), endPoint->getAbsolutePos());
       startPoint->draw(p);
       endPoint->draw(p);
    }
}

MovablePoint *GradientPoints::getPointAt(QPointF absPos)
{
    if(enabled) {
        if(startPoint->isPointAt(absPos) ) {
            return startPoint;
        } else if (endPoint->isPointAt(absPos) ){
            return endPoint;
        }
    }
    return NULL;
}

QPointF GradientPoints::getStartPoint()
{
    return startPoint->getAbsolutePos();
}

QPointF GradientPoints::getEndPoint()
{
    return endPoint->getAbsolutePos();
}

void GradientPoints::setColors(QColor startColor, QColor endColor)
{
    startPoint->setColor(startColor);
    endPoint->setColor(endColor);
}
