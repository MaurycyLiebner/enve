#ifndef SEGMENT_H
#define SEGMENT_H

class SmartNodePoint;
class SmartCtrlPoint;
class PathPointsHandler;

#include "smartPointers/stdpointer.h"
#include "Segments/qcubicsegment2d.h"
#include "pointhelpers.h"

class NormalSegment {
public:
    bool operator==(const NormalSegment& other) const;

    bool operator!=(const NormalSegment& other) const {
        return !this->operator==(other);
    }

    struct PointOnSegment {
        qreal fT;
        QPointF fPos;
    };
    struct SubSegment {
        SmartNodePoint* fFirstPt;
        SmartNodePoint* fLastPt;
        const NormalSegment* fParentSeg;

        bool isValid() const {
            return fFirstPt && fLastPt && fParentSeg;
        }

        qreal getMinT() const;
        qreal getMaxT() const;

        qreal getParentTAtThisT(const qreal& thisT) const;
        QPointF getRelPosAtT(const qreal &thisT) const;
    };
    NormalSegment();
    NormalSegment(SmartNodePoint * const firstNode,
                  SmartNodePoint * const lastNode,
                  PathPointsHandler * const handler);

    QPointF getRelPosAtT(const qreal &t) const;
    QPointF getAbsPosAtT(const qreal &t) const;

    void makePassThroughAbs(const QPointF &absPos, const qreal &t);
    void makePassThroughRel(const QPointF &relPos, const qreal &t);

    void finishPassThroughTransform();
    void startPassThroughTransform();
    void cancelPassThroughTransform();

    void generateSkPath();

    void drawHoveredSk(SkCanvas * const canvas,
                       const SkScalar &invScale);

    SmartNodePoint *getFirstNode() const;
    SmartNodePoint *getLastNode() const;

    QPointF getSlopeVector(const qreal &t);

    bool isValid() const;

    void reset() { clear(); }

    void clear();

    void disconnect() const;

    int nodesCount() const;

    int innerNodesCount() const {
        return mInnerDnD.count();
    }

    SmartNodePoint* getNodeAt(const int& id) const;

    SubSegment getClosestSubSegment(const QPointF& relPos,
                                    qreal& minDist) const {
        auto relSeg = getAsRelSegment();
        qreal closestT;
        minDist = relSeg.minDistanceTo(relPos, &closestT);
        return subSegmentAtT(closestT);
    }

    SubSegment getClosestSubSegmentForDummy(const QPointF& relPos,
                                            qreal& minDist) const;

    void afterChanged() const {
        updateDnDPos();
    }

    qCubicSegment2D getAsAbsSegment() const;
    qCubicSegment2D getAsRelSegment() const;

    void updateDnD();

    qreal closestRelT(const QPointF& relPos) const {
        return getAsRelSegment().tValueForPointClosestTo(relPos);
    }

    qreal closestAbsT(const QPointF& absPos) const {
        return getAsAbsSegment().tValueForPointClosestTo(absPos);
    }
private:
    void updateDnDPos() const;
    SubSegment subSegmentAtT(const qreal& t) const;

    SkPath mSkPath;
    stdptr<PathPointsHandler> mHandler_k;
    stdptr<SmartNodePoint> mFirstNode;
    stdptr<SmartCtrlPoint> mFirstNodeC2;
    QList<stdptr<SmartNodePoint>> mInnerDnD;
    stdptr<SmartCtrlPoint> mLastNodeC0;
    stdptr<SmartNodePoint> mLastNode;
};

#endif // SEGMENT_H
