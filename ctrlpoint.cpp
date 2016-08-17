#include "ctrlpoint.h"
#include "pathpoint.h"
#include "vectorpath.h"

CtrlPoint::CtrlPoint(QPointF absPos,
                     PathPoint *parentPoint, bool isStartCtrlPt) :
    MovablePoint(absPos,
                 parentPoint->getParentPath(),
                 MovablePointType::TYPE_CTRL_POINT)
{
    mIsStartCtrlPt = isStartCtrlPt;
    mParentPoint = parentPoint;
}

CtrlPoint::CtrlPoint(qreal relPosX, qreal relPosY,
                     PathPoint *parentPoint, bool isStartCtrlPt) :
    MovablePoint(relPosX, relPosY,
                 parentPoint->getParentPath(),
                 MovablePointType::TYPE_CTRL_POINT)
{
    mIsStartCtrlPt = isStartCtrlPt;
    mParentPoint = parentPoint;
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
    mOtherCtrlPt->MovablePoint::startTransform();
}

void CtrlPoint::finishTransform()
{
    startNewUndoRedoSet();
    MovablePoint::finishTransform();
    mOtherCtrlPt->MovablePoint::finishTransform();
    finishUndoRedoSet();
}

void CtrlPoint::setOtherCtrlPt(CtrlPoint *ctrlPt) {
    mOtherCtrlPt = ctrlPt;
}

void CtrlPoint::remove()
{
    mParentPoint->setCtrlPtEnabled(false, mIsStartCtrlPt);
}

void CtrlPoint::setRelativePos(QPointF relPos, bool saveUndoRedo)
{
    MovablePoint::setRelativePos(relPos, saveUndoRedo);
    mParentPoint->getParentPath()->schedulePathUpdate();
}
