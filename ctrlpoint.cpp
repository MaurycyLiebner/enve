#include "ctrlpoint.h"
#include "pathpoint.h"
#include "Boxes/vectorpath.h"
#include "Boxes/boxesgroup.h"

CtrlPoint::CtrlPoint(PathPoint *parentPoint, bool isStartCtrlPt) :
    MovablePoint(parentPoint->getParent(),
                 MovablePointType::TYPE_CTRL_POINT, 5.) {
    mIsStartCtrlPt = isStartCtrlPt;
    mParentPoint = parentPoint;
    mParent = mParentPoint->getParent();
    anim_setTraceKeyOnCurrentFrame(true);
}

void CtrlPoint::setIsStartCtrlPt(const bool &bT) {
    mIsStartCtrlPt = bT;
}

void CtrlPoint::moveToAbsWithoutUpdatingTheOther(QPointF absPos)
{
    MovablePoint::moveToAbs(absPos);
}

void CtrlPoint::moveToAbs(QPointF absPos)
{
    MovablePoint::moveToAbs(absPos);
    if(mOtherCtrlPt->isSelected()) {
        return;
    }
    mParentPoint->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::moveByRel(const QPointF &relTranslation)
{
    MovablePoint::moveByRel(relTranslation);
    if(mOtherCtrlPt->isSelected()) {
        return;
    }
    mParentPoint->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::startTransform() {
    MovablePoint::startTransform();
    mParentPoint->MovablePoint::startTransform();
    mOtherCtrlPt->MovablePoint::startTransform();
}

void CtrlPoint::finishTransform() {
    mParentPoint->MovablePoint::finishTransform();
    MovablePoint::finishTransform();
    mOtherCtrlPt->MovablePoint::finishTransform();
}

void CtrlPoint::cancelTransform() {
    mParentPoint->MovablePoint::cancelTransform();
    MovablePoint::finishTransform();
    mOtherCtrlPt->MovablePoint::cancelTransform();
}

void CtrlPoint::setOtherCtrlPt(CtrlPoint *ctrlPt) {
    mOtherCtrlPt = ctrlPt;
}

void CtrlPoint::removeFromVectorPath()
{
    mParentPoint->setCtrlPtEnabled(false, mIsStartCtrlPt);
}

bool CtrlPoint::isHidden()
{
    return MovablePoint::isHidden() ||
           (!mParentPoint->isNeighbourSelected() && !BoxesGroup::getCtrlsAlwaysVisible() );
}
