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

void GradientPoints::initialize(PathBox *parentT)
{
    parent = parentT;
    startPoint = new GradientPoint(parent);
    startPoint->incNumberPointers();
    addChildAnimator(startPoint->getRelativePosAnimatorPtr() );
    startPoint->getRelativePosAnimatorPtr()->setName("point1");
    endPoint = new GradientPoint(parent);
    endPoint->getRelativePosAnimatorPtr()->setName("point2");
    endPoint->incNumberPointers();
    addChildAnimator(endPoint->getRelativePosAnimatorPtr() );
    enabled = false;
}

void GradientPoints::loadFromSql(int fillGradientStartId, int fillGradientEndId)
{
    startPoint->loadFromSql(fillGradientStartId);
    startPoint->loadFromSql(fillGradientEndId);
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
