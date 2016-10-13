#include "gradientpoints.h"

GradientPoints::GradientPoints() : ComplexAnimator()
{
    setName("gradient points");
}

GradientPoints::~GradientPoints()
{
    startPoint->decNumberPointers();
    endPoint->decNumberPointers();
}

void GradientPoints::initialize(PathBox *parentT,
                                QPointF startPt, QPointF endPt)
{
    parent = parentT;
    startPoint = new GradientPoint(startPt, parent);
    startPoint->incNumberPointers();
    addChildAnimator(startPoint->getRelativePosAnimatorPtr() );
    startPoint->getRelativePosAnimatorPtr()->setName("point1");
    endPoint = new GradientPoint(endPt, parent);
    endPoint->getRelativePosAnimatorPtr()->setName("point2");
    endPoint->incNumberPointers();
    addChildAnimator(endPoint->getRelativePosAnimatorPtr() );
    enabled = false;
}

void GradientPoints::initialize(PathBox *parentT,
                                int fillGradientStartId, int fillGradientEndId)
{
    parent = parentT;
    startPoint = new GradientPoint(fillGradientStartId, parent);
    startPoint->incNumberPointers();
    addChildAnimator(startPoint->getRelativePosAnimatorPtr() );
    startPoint->getRelativePosAnimatorPtr()->setName("point1");
    endPoint = new GradientPoint(fillGradientEndId, parent);
    endPoint->incNumberPointers();
    endPoint->getRelativePosAnimatorPtr()->setName("point2");
    addChildAnimator(endPoint->getRelativePosAnimatorPtr() );
    enabled = false;
}

void GradientPoints::clearAll()
{
    delete startPoint;
    delete endPoint;
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

void GradientPoints::drawGradientPoints(QPainter *p)
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

void GradientPoints::attachToBoneFromSqlZId()
{
    startPoint->attachToBoneFromSqlZId();
    endPoint->attachToBoneFromSqlZId();
}
