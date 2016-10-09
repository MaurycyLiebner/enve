#include "pathpivot.h"
#include "canvas.h"

PathPivot::PathPivot(Canvas *parent) :
    MovablePoint(QPointF(0., 0.), parent,
                 MovablePointType::TYPE_PIVOT_POINT, 10.)
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
        p->setBrush(QColor(0, 255, 0));
    } else {
        p->setBrush(QColor(125, 255, 125));
    }
    p->setPen(QPen(Qt::black, 1.5));
    p->drawEllipse(absPos,
                   mRadius, mRadius);

    p->translate(absPos);
    qreal halfRadius = mRadius*0.5;
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
    mLastDRot = 0.;
    mRotHalfCycles = 0;
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
                                bool xOnly, bool yOnly,
                                bool inputTransformationEnabled,
                                qreal inputTransformationValue,
                                bool startTransform)
{
    if(mRotating) {
        QPointF absPos = getAbsolutePos();
        qreal rot;
        if(inputTransformationEnabled) {
            rot = inputTransformationValue;
        } else {
            QLineF dest_line(absPos, moveDestAbs);
            QLineF prev_line(absPos, pressPos);
            qreal d_rot = dest_line.angleTo(prev_line);
            if(d_rot > 180)
            {
                d_rot -= 360;
            }

            if(mLastDRot - d_rot > 90) {
                mRotHalfCycles += 2;
            } else if(mLastDRot - d_rot < -90) {
                mRotHalfCycles -= 2;
            }
            mLastDRot = d_rot;
            rot = d_rot + mRotHalfCycles*180.;
        }

        mCanvas->rotateBoxesBy(rot, absPos, startTransform);
        return true;
    } else if(mScaling) {
        QPointF absPos = getAbsolutePos();
        QPointF distMoved = moveDestAbs - pressPos;

        //if(scaleBy < 0.f) scaleBy = 0.f;
        qreal scaleBy;
        if(inputTransformationEnabled) {
            scaleBy = inputTransformationValue;
        } else {
            scaleBy = 1. + distSign(distMoved)*0.005;
        }
        if(xOnly) {
            mCanvas->scaleBoxesBy(scaleBy, 1., absPos, startTransform);
        } else if(yOnly) {
            mCanvas->scaleBoxesBy(1., scaleBy, absPos, startTransform);
        } else {
            if(isShiftPressed() || inputTransformationEnabled) {
                mCanvas->scaleBoxesBy(scaleBy, absPos, startTransform);
            } else {
                qreal scaleXBy = 1. +
                        distSign(distMoved - QPointF(0., distMoved.y()) )*0.005;
                qreal scaleYBy = 1. +
                        distSign(distMoved - QPointF(distMoved.x(), 0.) )*0.005;
                mCanvas->scaleBoxesBy(scaleXBy, scaleYBy, absPos,
                                      startTransform);
            }
        }
        return true;
    }
    return false;
}

//bool PathPivot::isRotationPathAt(QPointF absPos)
//{
//    return mMappedRotationPath.contains(absPos);
//}
