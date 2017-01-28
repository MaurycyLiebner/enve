#include "pathpivot.h"
#include "canvas.h"

PathPivot::PathPivot(Canvas *parent) :
    MovablePoint(parent, MovablePointType::TYPE_PIVOT_POINT, 7.)
{
    mCanvas = parent;
//    mRotationPath.addEllipse(QPointF(0., 0.), 50., 50.);
//    QPainterPath removeEllipse;
//    removeEllipse.addEllipse(QPointF(0., 0.), 40., 40.);
//    mRotationPath -= removeEllipse;
}

void PathPivot::startTransform() {
    mRelPos.setTransformed(false);
    MovablePoint::startTransform();
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
    if(mHidden) return false;
    if(isPointAtAbsPos(absPressPos)) {
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
    if(val > 0.) {
        mult = 1;
    } else {
        mult = -1;
    }
    return val*val*mult;
}

qreal distSign(QPointF distPt) {
    qreal val = signedSquare(distPt.x()) + signedSquare(distPt.y() );
    if(val > 0.) {
        return sqrt(val);
    } else {
        return -sqrt(-val);
    }
}

bool PathPivot::handleMouseMove(QPointF moveDestAbs, QPointF pressPos,
                                bool xOnly, bool yOnly,
                                bool inputTransformationEnabled,
                                qreal inputTransformationValue,
                                bool startTransform, CanvasMode mode)
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

        if(mode == CanvasMode::MOVE_PATH) {
            mCanvas->rotateBoxesBy(rot, absPos, startTransform);
        } else {
            mCanvas->rotatePointsBy(rot, absPos, startTransform);
        }
        return true;
    } else if(mScaling) {
        QPointF absPos = getAbsolutePos();
        QPointF distMoved = moveDestAbs - pressPos;

        //if(scaleBy < 0.) scaleBy = 0.;
        qreal scaleBy;
        if(inputTransformationEnabled) {
            scaleBy = inputTransformationValue;
        } else {
            scaleBy = 1. + distSign(distMoved)*0.005;
        }
        qreal scaleX;
        qreal scaleY;
        if(xOnly) {
            scaleX = scaleBy;
            scaleY = 1.;
        } else if(yOnly) {
            scaleX = 1.;
            scaleY = scaleBy;
        } else {
            if(isShiftPressed() || inputTransformationEnabled) {
                scaleX = scaleBy;
                scaleY = scaleBy;
            } else {
                scaleX = 1. +
                        distSign(distMoved - QPointF(0., distMoved.y()) )*0.005;
                scaleY = 1. +
                        distSign(distMoved - QPointF(distMoved.x(), 0.) )*0.005;
            }
        }

        if(mode == CanvasMode::MOVE_PATH) {
            mCanvas->scaleBoxesBy(scaleX, scaleY, absPos, startTransform);
        } else {
            mCanvas->scalePointsBy(scaleX, scaleY, absPos, startTransform);
        }
        return true;
    }
    return false;
}

//bool PathPivot::isRotationPathAt(QPointF absPos)
//{
//    return mMappedRotationPath.contains(absPos);
//}
