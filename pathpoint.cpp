#include "pathpoint.h"
#include "vectorpath.h"
#include <QPainter>

const qreal PathPoint::RADIUS = 10.f;

PathPoint::PathPoint(QPointF absPos, VectorPath *vectorPath)
{
    mVectorPath = vectorPath;
    setAbsolutePos(absPos);
}

void PathPoint::setAbsolutePos(QPointF pos)
{
    setRelativePos(mVectorPath->getCombinedTransform().inverted().map(pos));
}

void PathPoint::setRelativePos(QPointF pos)
{
    mRelativePos.setCurrentValue(pos, false);
    mVectorPath->updatePath();
}

QPointF PathPoint::getRelativePos()
{
    return mRelativePos.getCurrentValue();
}

QPointF PathPoint::getAbsolutePos()
{
    return mVectorPath->getCombinedTransform().map(getRelativePos());
}

void PathPoint::setStartCtrlPt(QPointF startCtrlPt)
{
    mStartCtrlPt.setCurrentValue(startCtrlPt, false);
    mVectorPath->updatePath();
}

QPointF PathPoint::getStartCtrlPt()
{
    return mStartCtrlPt.getCurrentValue();
}

void PathPoint::setEndCtrlPt(QPointF endCtrlPt)
{
    mEndCtrlPt.setCurrentValue(endCtrlPt, false);
    mVectorPath->updatePath();
}

QPointF PathPoint::getEndCtrlPt()
{
    return mEndCtrlPt.getCurrentValue();
}

void PathPoint::draw(QPainter *p)
{
    if(mSelected) {
        p->setBrush(QColor(0, 0, 255, 75));
    } else {
        p->setBrush(Qt::NoBrush);
    }
    p->drawEllipse(getAbsolutePos(),
                   RADIUS, RADIUS);
}

PathPoint* PathPoint::getNextPoint()
{
    return mNextPoint;
}

PathPoint *PathPoint::getPreviousPoint()
{
    return mPreviousPoint;
}

void PathPoint::setNextPoint(PathPoint *nextPoint)
{
    mNextPoint = nextPoint;
}

void PathPoint::setPreviousPoint(PathPoint *previousPoint)
{
    mPreviousPoint = previousPoint;
}

bool PathPoint::hasNextPoint() {
    return mNextPoint != NULL;
}

bool PathPoint::hasPreviousPoint() {
    return mPreviousPoint != NULL;
}

void PathPoint::setPointAsNext(PathPoint *pointToSet) {
    setNextPoint(pointToSet);
    pointToSet->setPreviousPoint(this);
    mVectorPath->updatePath();
}

void PathPoint::setPointAsPrevious(PathPoint *pointToSet) {
    setPreviousPoint(pointToSet);
    pointToSet->setNextPoint(this);
    mVectorPath->updatePath();
}

void PathPoint::setPointAsNextOrPrevious(PathPoint *pointToSet) {
    if(!hasNextPoint()) {
        setPointAsNext(pointToSet);
    } else if(!hasPreviousPoint()) {
        setPointAsPrevious(pointToSet);
    }

}

VectorPath *PathPoint::getParentPath()
{
    return mVectorPath;
}

PathPoint *PathPoint::addPoint(QPointF absPos)
{
    return mVectorPath->addPoint(absPos, this);
}

bool PathPoint::isEndPoint() {
    return mNextPoint == NULL || mPreviousPoint == NULL;
}

bool PathPoint::isPointAt(QPointF absPoint)
{
    QPointF dist = getAbsolutePos() - absPoint;
    return (dist.x()*dist.x() + dist.y()*dist.y() < RADIUS*RADIUS);
}

bool PathPoint::isContainedInRect(QRectF absRect)
{
    return absRect.contains(getAbsolutePos());
}

void PathPoint::moveBy(QPointF absTranslation) {
    setAbsolutePos(getAbsolutePos() + absTranslation);
}

void PathPoint::select()
{
    mSelected = true;
}

void PathPoint::deselect()
{
    mSelected = false;
}

bool PathPoint::isSelected()
{
    return mSelected;
}
