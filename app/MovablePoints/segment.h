#ifndef SEGMENT_H
#define SEGMENT_H

class SmartNodePoint;
class SmartCtrlPoint;
#include "smartPointers/stdpointer.h"
#include "Segments/qcubicsegment2d.h"

struct PointOnSegment {
    qreal fT;
    QPointF fPos;
};

class Segment {
public:
    Segment();
    Segment(SmartNodePoint * const pt1, SmartNodePoint * const pt2);

    QPointF getRelPosAtT(const qreal &t) const;
    QPointF getAbsPosAtT(const qreal &t) const;

    void makePassThroughAbs(const QPointF &absPos, const qreal &t);
    void makePassThroughRel(const QPointF &relPos, const qreal &t);

    void finishPassThroughTransform();
    void startPassThroughTransform();
    void cancelPassThroughTransform();

    void setEditPath(const bool &bT);

    void generateSkPath();

    void drawHoveredSk(SkCanvas * const canvas,
                       const SkScalar &invScale);

    SmartNodePoint *getPoint1() const;
    SmartNodePoint *getPoint2() const;

    QPointF getSlopeVector(const qreal &t);

    bool isValid() const {
        return mPoint1 != nullptr && mPoint2 != nullptr &&
               mPoint1C2Pt != nullptr && mPoint2C0Pt != nullptr;
    }

    void setPoint1(SmartNodePoint * const pt1);
    void setPoint2(SmartNodePoint * const pt2);
private:
    qCubicSegment2D getAsAbsSegment() const;
    qCubicSegment2D getAsRelSegment() const;

    SkPath mSkPath;

    SmartNodePoint* mPoint1;
    SmartCtrlPoint* mPoint1C2Pt;
    SmartCtrlPoint* mPoint2C0Pt;
    SmartNodePoint* mPoint2;
};

#endif // SEGMENT_H
