#ifndef CTRLPOINT_H
#define CTRLPOINT_H
#include "movablepoint.h"

class CtrlPoint : public MovablePoint
{
public:
    CtrlPoint(PathPoint *parentPoint, bool isStartCtrlPt);
    void moveToAbsWithoutUpdatingTheOther(QPointF absPos);
    void moveToAbs(QPointF absPos);
    //void moveByWithoutUpdatingTheOther(QPointF absTranslation);
    void moveByRel(QPointF relTranslation);
    void startTransform();
    void finishTransform();
    void setOtherCtrlPt(CtrlPoint *ctrlPt);
    void removeFromVectorPath();

    bool isHidden();
    void setIsStartCtrlPt(const bool &bT);
    void cancelTransform();
private:
    bool mIsStartCtrlPt;
    PathPoint *mParentPoint;
    CtrlPoint *mOtherCtrlPt;
};

#endif // CTRLPOINT_H
