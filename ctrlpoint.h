#ifndef CTRLPOINT_H
#define CTRLPOINT_H
#include "movablepoint.h"

class CtrlPoint : public NonAnimatedMovablePoint {
public:
    CtrlPoint(NodePoint *parentPoint, bool isStartCtrlPt);

    QPointF getRelativePos() const;

    void moveByAbs(const QPointF &absTranslatione);
    void setRelativePos(const QPointF &relPos);
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
    void scale(const qreal &sx, const qreal &sy);

    int getPtId();

    void setRelativePosVal(const QPointF &relPos);
private:
    bool mIsStartCtrlPt;
    NodePoint *mParentPoint;
    CtrlPoint *mOtherCtrlPt;
};

#endif // CTRLPOINT_H
