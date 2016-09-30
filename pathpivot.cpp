#include "pathpivot.h"
#include "canvas.h"

PathPivot::PathPivot(Canvas *parent) :
    MovablePoint(QPointF(0.f, 0.f), parent,
                 MovablePointType::TYPE_PIVOT_POINT, 10.f)
{
    mCanvas = parent;
//    mRotationPath.addEllipse(QPointF(0.f, 0.f), 50.f, 50.f);
//    QPainterPath removeEllipse;
//    removeEllipse.addEllipse(QPointF(0.f, 0.f), 40.f, 40.f);
//    mRotationPath -= removeEllipse;
}

void PathPivot::draw(QPainter *p)
{
    if(mHidden) {
        return;
    }
    p->save();
    QPointF absPos = getAbsolutePos();
//    p->save();
//    p->setBrush(Qt::red);
//    p->drawPath(mMappedRotationPath);
//    p->restore();

    if(mSelected) {
        p->setBrush(QColor(0, 255, 0, 155));
    } else {
        p->setBrush(QColor(0, 255, 0, 75));
    }
    p->setPen(QPen(Qt::black, 1.f));
    p->drawEllipse(absPos,
                   mRadius, mRadius);

    p->translate(absPos);
    qreal halfRadius = mRadius*0.5f;
    p->drawLine(QPointF(-halfRadius, 0), QPointF(halfRadius, 0));
    p->drawLine(QPointF(0, -halfRadius), QPointF(0, halfRadius));
    p->restore();
}

//void PathPivot::updateRotationMappedPath() {
//    mMappedRotationPath = mRotationPath.translated(getAbsolutePos());
//    
//}

void PathPivot::finishTransform()
{
    if(!mTransformStarted) {
        return;
    }
    mTransformStarted = false;
    mCanvas->setPivotPositionForSelected();
}

void PathPivot::setRelativePos(QPointF relPos, bool saveUndoRedo)
{
    MovablePoint::setRelativePos(relPos, saveUndoRedo);
//    updateRotationMappedPath();
    
}

bool PathPivot::isRotating()
{
    return mRotating;
}

bool PathPivot::isScaling()
{
    return mScaling;
}

void PathPivot::startRotating() {
    mRotating = true;
}

void PathPivot::startScaling()
{
    mScaling = true;
}

bool PathPivot::handleMousePress(QPointF absPressPos)
{
    if(isPointAt(absPressPos)) {
        select();
        return true;
    }/* else {
        if(isRotationPathAt(absPressPos) ) {
            startRotating();
            return true;
        }
    }*/
    return false;
}

bool PathPivot::handleMouseRelease()
{
    if(mRotating) {
        mRotating = false;
        return true;
    } else if(mScaling) {
        mScaling = false;
        return true;
    }
    return false;
}

qreal signedSquare(qreal val) {
    int mult;
    if(val > 0.f) {
        mult = 1;
    } else {
        mult = -1;
    }
    return val*val*mult;
}

qreal distSign(QPointF distPt) {
    qreal val = signedSquare(distPt.x()) + signedSquare(distPt.y() );
    if(val > 0.f) {
        return sqrt(val);
    } else {
        return -sqrt(-val);
    }
}

bool PathPivot::handleMouseMove(QPointF moveDestAbs, QPointF pressPos,
                                bool startTransform)
{
    if(mRotating) {
        QPointF absPos = getAbsolutePos();
        QLineF dest_line(absPos, moveDestAbs);
        QLineF prev_line(absPos, pressPos);
        qreal d_rot = dest_line.angleTo(prev_line);
        if(d_rot > 180)
        {
            d_rot -= 360;
        }
        mCanvas->rotateBoxesBy(d_rot, absPos, startTransform);
        return true;
    } else if(mScaling) {
        QPointF absPos = getAbsolutePos();
        QPointF distMoved = moveDestAbs - pressPos;
        qreal scaleBy = 1.f + distSign(distMoved)*0.005f;
        if(scaleBy < 0.f) scaleBy = 0.f;
        mCanvas->scaleBoxesBy(scaleBy, absPos, startTransform);
        return true;
    }
    return false;
}

//bool PathPivot::isRotationPathAt(QPointF absPos)
//{
//    return mMappedRotationPath.contains(absPos);
//}
