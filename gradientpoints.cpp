#include "gradientpoints.h"

GradientPoints::GradientPoints() : ComplexAnimator()
{
    prp_setName("gradient points");
}

GradientPoints::~GradientPoints()
{
    startPoint->decNumberPointers();
    endPoint->decNumberPointers();
}

void GradientPoints::initialize(PathBox *parentT)
{
    parent = parentT;
    startPoint = new GradientPoint(parent);
    startPoint->incNumberPointers();
    addChildAnimator(startPoint->getRelativePosAnimatorPtr() );
    startPoint->getRelativePosAnimatorPtr()->prp_setName("point1");
    endPoint = new GradientPoint(parent);
    endPoint->getRelativePosAnimatorPtr()->prp_setName("point2");
    endPoint->incNumberPointers();
    addChildAnimator(endPoint->getRelativePosAnimatorPtr() );
    enabled = false;
}

void GradientPoints::loadFromSql(int fillGradientStartId, int fillGradientEndId)
{
    startPoint->loadFromSql(fillGradientStartId);
    startPoint->loadFromSql(fillGradientEndId);
}

void GradientPoints::duplicatePointsFrom(GradientPoint *startPointT,
                                         GradientPoint *endPointT) {
    startPointT->makeDuplicate(startPoint);
    endPointT->makeDuplicate(endPoint);
}

void GradientPoints::makeDuplicate(QrealAnimator *target) {
    GradientPoints *gradientPointsTarget = (GradientPoints*)target;
    gradientPointsTarget->duplicatePointsFrom(startPoint,
                                              endPoint);
}

void GradientPoints::enable()
{
    if(enabled) {
        return;
    }
    enabled = true;
}

void GradientPoints::setPositions(QPointF startPos,
                                  QPointF endPos, bool saveUndoRedo) {
    startPoint->setRelativePos(startPos, saveUndoRedo);
    endPoint->setRelativePos(endPos, saveUndoRedo);
}

void GradientPoints::disable()
{
    enabled = false;
}

void GradientPoints::drawGradientPoints(QPainter *p) {
    if(enabled) {
        p->setPen(QPen(Qt::black, 1.5));
        p->drawLine(startPoint->getAbsolutePos(), endPoint->getAbsolutePos());
        p->setPen(QPen(Qt::white, 0.75));
        p->drawLine(startPoint->getAbsolutePos(), endPoint->getAbsolutePos());
        startPoint->draw(p);
        endPoint->draw(p);
    }
}

MovablePoint *GradientPoints::qra_getPointAt(const QPointF &absPos)
{
    if(enabled) {
        if(startPoint->isPointAtAbsPos(absPos) ) {
            return startPoint;
        } else if (endPoint->isPointAtAbsPos(absPos) ){
            return endPoint;
        }
    }
    return NULL;
}

QPointF GradientPoints::getStartPoint()
{
    return startPoint->getRelativePos();
}

QPointF GradientPoints::getEndPoint()
{
    return endPoint->getRelativePos();
}

void GradientPoints::setColors(QColor startColor, QColor endColor)
{
    startPoint->setColor(startColor);
    endPoint->setColor(endColor);
}
