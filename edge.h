#ifndef EDGE_H
#define EDGE_H
#include <QPainter>
#include "selfref.h"
#include "pointhelpers.h"
class BonePoint;
class PathPoint;
class CtrlPoint;

class Edge : public StdSelfRef {
public:
    Edge(PathPoint *pt1, PathPoint *pt2);

    static void getNewRelPosForKnotInsertionAtT(const QPointF &P0,
                                                QPointF *P1_ptr,
                                                QPointF *P2_ptr,
                                                QPointF P3,
                                                QPointF *new_p_ptr,
                                                QPointF *new_p_start_ptr,
                                                QPointF *new_p_end_ptr,
                                                const qreal &t);

    static QPointF getPosBetweenPointsAtT(const qreal &t,
                                          const QPointF &p0Pos,
                                          const QPointF &p1EndPos,
                                          const QPointF &p2StartPos,
                                          const QPointF &p3Pos);

    static qreal getLength(const QPointF &p0Pos,
                           const QPointF &p1EndPos,
                           const QPointF &p2StartPos,
                           const QPointF &p3Pos,
                           int divisions);

    static qreal getLength(const QPointF &p0Pos,
                           const QPointF &p1EndPos,
                           const QPointF &p2StartPos,
                           const QPointF &p3Pos);

    static QPointF getRelPosBetweenPointsAtT(const qreal &t,
                                             PathPoint *point1,
                                             PathPoint *point2);
    static QPointF getAbsPosBetweenPointsAtT(const qreal &t,
                                             PathPoint *point1,
                                             PathPoint *point2);

    QPointF getRelPosAtT(const qreal &t);
    QPointF getAbsPosAtT(const qreal &t);

    void makePassThrough(const QPointF &absPos);

    void finishPassThroughTransform();

    void startPassThroughTransform();

    void setEditPath(const bool &bT);

    void generatePainterPath();

    void drawHover(QPainter *p);

    PathPoint *getPoint1() const;

    PathPoint *getPoint2() const;

    void setPoint1(PathPoint *point1);

    void setPoint2(PathPoint *point2);

    void setPressedT(const qreal &t);

    void getNearestAbsPosAndT(const QPointF &absPos,
                              QPointF *nearestPoint,
                              qreal *t);

    QPointF getSlopeVector(const qreal &t);
private:
    QPainterPath mPath;



    PathPoint *mPoint1;
    CtrlPoint *mPoint1EndPt;
    PathPoint *mPoint2;
    CtrlPoint *mPoint2StartPt;
    bool mEditPath = true;

    qreal mPressedT;
};

struct BoneInfluencePoint {
    BoneInfluencePoint() {}
    BoneInfluencePoint(const qreal &tT) {
        weight = 0.;
        t = tT;
    }

    BoneInfluencePoint(const qreal &tT,
                       const qreal &weightT) {
        weight = weightT;
        t = tT;
    }

    qreal t;
    qreal weight;
    QPointF absPos;
};

class BoneEdgeInfluencePoints {
public:
    BoneEdgeInfluencePoints(BonePoint *bonePoint) {
        mBonePoint = bonePoint;
    }

    void addInfluencePointAtAbsPos(const QPointF &absPos) {
        qreal tAtPos;
        QPointF nearestPoint;
        mParentEdge->getNearestAbsPosAndT(absPos,
                                          &nearestPoint,
                                          &tAtPos);
        if(pointToLen(nearestPoint - absPos) > 10.) return;
        removeInfluencePointNearAbsPos(absPos);
        addInfluencePoint(tAtPos );
    }

    void addInfluencePoint(const qreal &t) {
        mBoneInfluencePoints << BoneInfluencePoint(t);
    }

    void removeInfluencePointNearAbsPos(const QPointF &absPos) {
        qreal leastDist = 10000.;
        int bestId = -1;
        for(int i = 0; i < mBoneInfluencePoints.count(); i++) {
            const BoneInfluencePoint &boneInfPt = mBoneInfluencePoints.at(i);
            qreal thisDist =
                    pointToLen(absPos - mParentEdge->getAbsPosAtT(boneInfPt.t));
            if(leastDist > thisDist) {
                leastDist = thisDist;
                bestId = i;
            }
        }
        if(bestId == -1 || leastDist > 10.) return;
        removeInfluencePoint(bestId);
    }

    void removeInfluencePoint(const int &id) {
        mBoneInfluencePoints.removeAt(id);
    }

    void updatePointsAbsPosition() {
        for(int i = 0; i < mBoneInfluencePoints.count(); i++) {
            mBoneInfluencePoints[i].absPos =
                    mParentEdge->getAbsPosAtT(mBoneInfluencePoints[i].t);
        }
    }

private:
    Edge *mParentEdge;
    qreal mPoint1Weight;
    qreal mPoint2Weight;
    QList<BoneInfluencePoint> mBoneInfluencePoints;
    BonePoint *mBonePoint;
};
#endif // EDGE_H
