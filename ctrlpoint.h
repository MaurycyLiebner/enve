#ifndef CTRLPOINT_H
#define CTRLPOINT_H
#include "movablepoint.h"

class CtrlPoint : public MovablePoint {
public:
    CtrlPoint(PathPoint *parentPoint, bool isStartCtrlPt);

    QPointF getRelativePos() const;

    void moveByAbs(const QPointF &absTranslatione);
    void setRelativePos(const QPointF &relPos,
                        const bool &saveUndoRedo = true);
    void moveToAbsWithoutUpdatingTheOther(const QPointF &absPos);
    void moveToAbs(QPointF absPos);
    //void moveByWithoutUpdatingTheOther(QPointF absTranslation);
    void moveByRel(const QPointF &relTranslation);
    void startTransform();
    void finishTransform();
    void setOtherCtrlPt(CtrlPoint *ctrlPt);
    void removeFromVectorPath();

    bool isHidden();
    void setIsStartCtrlPt(const bool &bT);
    void cancelTransform();

    void rotate(const qreal &rotate);
    QPointF getRelativePosAtRelFrame(const int &frame) const;
private:
    bool mIsStartCtrlPt;
    PathPoint *mParentPoint;
    CtrlPoint *mOtherCtrlPt;
};

#endif // CTRLPOINT_H
