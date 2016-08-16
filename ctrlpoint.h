#ifndef CTRLPOINT_H
#define CTRLPOINT_H
#include "movablepoint.h"

class CtrlPoint : public MovablePoint
{
public:
    CtrlPoint(QPointF absPos, PathPoint *parentPoint, bool isStartCtrlPt);
    void moveToWithoutUpdatingTheOther(QPointF absPos);
    void moveToAbs(QPointF absPos);
    void moveByWithoutUpdatingTheOther(QPointF absTranslation);
    void moveBy(QPointF absTranslation);
    void startTransform();
    void finishTransform();
    void setOtherCtrlPt(CtrlPoint *ctrlPt);
    void remove();
    void setRelativePos(QPointF relPos, bool saveUndoRedo = true);
private:
    bool mIsStartCtrlPt;
    PathPoint *mParentPoint;
    CtrlPoint *mOtherCtrlPt;
};

#endif // CTRLPOINT_H
