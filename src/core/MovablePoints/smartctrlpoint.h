#ifndef SMARTCTRLPOINT_H
#define SMARTCTRLPOINT_H
#include "MovablePoints/nonanimatedmovablepoint.h"
class SmartNodePoint;
class SmartCtrlPoint : public NonAnimatedMovablePoint {
    e_OBJECT
public:
    enum Type : char { C0, C2 };
protected:
    SmartCtrlPoint(SmartNodePoint * const parentPoint,
                   const Type &type);
public:
    void setRelativePos(const QPointF &relPos);
    void rotateRelativeToSavedPivot(const qreal rotate);

    void startTransform();
    void finishTransform();
    void cancelTransform();

    void remove();

    bool isVisible(const CanvasMode mode) const;
    void scale(const qreal sx, const qreal sy);

    bool enabled() const;

    void setOtherCtrlPt(SmartCtrlPoint * const ctrlPt);
private:
    const Type mCtrlType;
    SmartNodePoint* const mParentPoint_k;
    stdptr<SmartCtrlPoint> mOtherCtrlPt_cv;
};

#endif // SMARTCTRLPOINT_H
