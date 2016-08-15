#include "pathpivot.h"
#include "canvas.h"

PathPivot::PathPivot(Canvas *parent) :
    MovablePoint(QPointF(0.f, 0.f), parent,
                 MovablePointType::TYPE_PIVOT_POINT, 10.f)
{
    mCanvas = parent;
    mRotationPath.addEllipse(QPointF(0.f, 0.f), 50.f, 50.f);
    QPainterPath removeEllipse;
    removeEllipse.addEllipse(QPointF(0.f, 0.f), 40.f, 40.f);
    mRotationPath -= removeEllipse;
}

void PathPivot::draw(QPainter *p)
{
    if(mHidden) {
        return;
    }
    QPointF absPos = getAbsolutePos();
    p->save();
    p->setBrush(Qt::red);
    p->drawPath(mMappedRotationPath);
    p->restore();

    if(mSelected) {
        p->setBrush(QColor(0, 255, 0, 155));
    } else {
        p->setBrush(QColor(0, 255, 0, 75));
    }
    p->drawEllipse(absPos,
                   mRadius, mRadius);
    p->save();
    p->translate(absPos);
    qreal halfRadius = mRadius*0.5f;
    p->drawLine(QPointF(-halfRadius, 0), QPointF(halfRadius, 0));
    p->drawLine(QPointF(0, -halfRadius), QPointF(0, halfRadius));
    p->restore();
}

void PathPivot::updateRotationMappedPath() {
    mMappedRotationPath = mRotationPath.translated(getAbsolutePos());
    scheduleRepaint();
}

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
    updateRotationMappedPath();
}

bool PathPivot::isRotating()
{
    return mRotating;
}

bool PathPivot::handleMousePress(QPointF absPressPos)
{
    if(isPointAt(absPressPos)) {
        select();
        return true;
    } else {
        if(isRotationPathAt(absPressPos) ) {
            mRotating = true;
            return true;
        }
    }
    return false;
}

bool PathPivot::handleMouseRelease()
{
    if(mRotating) {
        mRotating = false;
        return true;
    }
    return false;
}

bool PathPivot::handleMouseMove(QPointF moveDestAbs, QPointF moveBy,
                                bool startTransform)
{
    if(mRotating) {
        QPointF absPos = getAbsolutePos();
        QLineF dest_line(absPos, moveDestAbs);
        QLineF prev_line(absPos, moveDestAbs - moveBy);
        qreal d_rot = dest_line.angleTo(prev_line);
        if(d_rot > 180)
        {
            d_rot -= 360;
        }
        mCanvas->rotateBoxesBy(d_rot, absPos, startTransform);
        return true;
    }
    return false;
}

bool PathPivot::isRotationPathAt(QPointF absPos)
{
    return mMappedRotationPath.contains(absPos);
}
