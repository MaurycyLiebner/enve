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
    NormalSegment(PathPointsHandler * const handler);
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

    bool isValid() const {
        return mFirstNode && mLastNode && mFirstNodeC2 && mLastNodeC0;
    }

    void setFirstNode(SmartNodePoint * const firstNode);
    void setLastNode(SmartNodePoint * const lastNode);

    void disconnect() const;

    int nodesCount() const {
        return mInnerDnD.count() + (mFirstNode ? 1 : 0) + (mLastNode ? 1 : 0);
    }

    int innerNodesCount() const {
        return mInnerDnD.count();
    }

    SmartNodePoint* getNodeAt(const int& id) const {
        if(id < 0 || id >= nodesCount()) return nullptr;
        if(id == 0) return mFirstNode;
        const int innerId = id - 1;
        if(innerId < mInnerDnD.count()) mInnerDnD.at(innerId);
        return mLastNode;
    }

    SubSegment getClosestSubSegment(const QPointF& relPos,
                                    qreal& minDist) const {
        auto relSeg = getAsRelSegment();
        qreal closestT;
        minDist = relSeg.minDistanceTo(relPos, &closestT);
        return subSegmentAtT(closestT);
    }

    void afterChanged() const {
        updateDnDPos();
    }
private:
    void updateDnDPos() const;

    qCubicSegment2D getAsAbsSegment() const;
    qCubicSegment2D getAsRelSegment() const;
    void updateInnerDnD();
    SubSegment subSegmentAtT(const qreal& t) const;

    SkPath mSkPath;
    const stdptr<PathPointsHandler> mHandler_k;
    SmartNodePoint* mFirstNode;
    SmartCtrlPoint* mFirstNodeC2;
    QList<SmartNodePoint*> mInnerDnD;
    SmartCtrlPoint* mLastNodeC0;
    SmartNodePoint* mLastNode;
};

#endif // SEGMENT_H
