#ifndef CTRLPOINT_H
#define CTRLPOINT_H
#include "movablepoint.h"

class CtrlPoint : public NonAnimatedMovablePoint {
    friend class StdSelfRef;
public:
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

    bool isHidden() const;
    void setIsStartCtrlPt(const bool &bT);
    void cancelTransform();

    void rotate(const qreal &rotate);
    void scale(const qreal &sx, const qreal &sy);

    int getPtId() const;

    void setRelativePosVal(const QPointF &relPos);
protected:
    CtrlPoint(NodePoint *parentPoint, bool isStartCtrlPt);
private:
    bool mIsStartCtrlPt;
    NodePoint* const mParentPoint_k;
    stdptr<CtrlPoint>mOtherCtrlPt_cv;
};

#endif // CTRLPOINT_H
