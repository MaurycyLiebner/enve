#include "ctrlpoint.h"
#include "pathpoint.h"
#include "Boxes/vectorpath.h"
#include "Boxes/boxesgroup.h"

CtrlPoint::CtrlPoint(PathPoint *parentPoint, bool isStartCtrlPt) :
    MovablePoint(parentPoint->getParentPath(),
                 MovablePointType::TYPE_CTRL_POINT, 5.)
{
    mIsStartCtrlPt = isStartCtrlPt;
    mParentPoint = parentPoint;
    mRelPos.setTraceKeyOnCurrentFrame(true);
}

void CtrlPoint::moveToWithoutUpdatingTheOther(QPointF absPos)
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

void CtrlPoint::moveBy(QPointF absTranslation)
{
    MovablePoint::moveBy(absTranslation);
    if(mOtherCtrlPt->isSelected()) {
        return;
    }
    mParentPoint->ctrlPointPosChanged(mIsStartCtrlPt);
}

void CtrlPoint::startTransform()
{
    MovablePoint::startTransform();
    mParentPoint->MovablePoint::startTransform();
    mOtherCtrlPt->MovablePoint::startTransform();
}

void CtrlPoint::finishTransform()
{
    mParentPoint->MovablePoint::finishTransform();
    MovablePoint::finishTransform();
    mOtherCtrlPt->MovablePoint::finishTransform();
}

void CtrlPoint::setOtherCtrlPt(CtrlPoint *ctrlPt) {
    mOtherCtrlPt = ctrlPt;
}

void CtrlPoint::remove()
{
    mParentPoint->setCtrlPtEnabled(false, mIsStartCtrlPt);
}

bool CtrlPoint::isHidden()
{
    return MovablePoint::isHidden() ||
           (!mParentPoint->isNeighbourSelected() && !BoxesGroup::getCtrlsAlwaysVisible() );
}
