#ifndef SMARTCTRLPOINT_H
#define SMARTCTRLPOINT_H
#include "nonanimatedmovablepoint.h"
class SmartNodePoint;
class SmartCtrlPoint : public NonAnimatedMovablePoint {
    friend class StdSelfRef;
public:
    enum Type { C0, C2 };
    void setRelativePos(const QPointF &relPos);
    void rotateRelativeToSavedPivot(const qreal &rotate);

    void startTransform();
    void finishTransform();
    void cancelTransform();

    void remove();

    bool isHidden() const;
    void scale(const qreal &sx, const qreal &sy);

    void setOtherCtrlPt(SmartCtrlPoint * const ctrlPt);
protected:
    SmartCtrlPoint(SmartNodePoint * const parentPoint,
                   const Type &type);
private:
    const Type mCtrlType;
    SmartNodePoint* const mParentPoint_k;
    stdptr<SmartCtrlPoint> mOtherCtrlPt_cv;
};

#endif // SMARTCTRLPOINT_H
