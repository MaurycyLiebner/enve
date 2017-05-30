#ifndef EDGE_H
#define EDGE_H
#include <QPainter>
#include "selfref.h"
class BonePoint;
class PathPoint;
class CtrlPoint;
#include "skiaincludes.h"

class VectorPathEdge : public StdSelfRef {
public:
    VectorPathEdge(PathPoint *pt1, PathPoint *pt2);

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

    void drawHovered(QPainter *p);
    void drawHoveredSk(SkCanvas *canvas,
                                 const SkScalar &invScale);

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
    SkPath mSkPath;

    PathPoint *mPoint1;
    CtrlPoint *mPoint1EndPt;
    PathPoint *mPoint2;
    CtrlPoint *mPoint2StartPt;
    bool mEditPath = true;

    qreal mPressedT;
};

class BoneInfluencePoint {
public:
    BoneInfluencePoint() {}
    BoneInfluencePoint(const qreal &tT) {
        mWeight = 0.;
        mT = tT;
    }

    BoneInfluencePoint(const qreal &tT,
                       const qreal &weightT) {
        mWeight = weightT;
        mT = tT;
    }

    const qreal &getT() const {
        return mT;
    }

    const qreal &getWeight() const {
        return mWeight;
    }

    const QPointF &getAbsPos() const {
        return mAbsPos;
    }

    void setAbsPos(const QPointF &absPos) {
        mAbsPos = absPos;
    }

private:
    qreal mAssignedRotation;
    QPointF mAssignedScale;
    QPointF mAssignedTranslation;

    qreal mCurrentRotation;
    QPointF mCurrentScale;
    QPointF mCurrentTranslation;

    qreal mT;
    qreal mWeight;
    QPointF mAbsPos;
};

class EdgeInfluencePoints {
public:
    EdgeInfluencePoints(BonePoint *bonePoint);

    void addInfluencePointAtAbsPos(const QPointF &absPos);

    void addInfluencePoint(const qreal &t);

    void removeInfluencePointNearAbsPos(const QPointF &absPos);

    void removeInfluencePoint(const int &id);

    void updatePointsAbsPosition();

private:
    VectorPathEdge *mParentEdge;
    BoneInfluencePoint mPoint1InfluencePoint;
    BoneInfluencePoint mPoint2InfluencePoint;
    QList<BoneInfluencePoint> mBoneInfluencePoints;

    BonePoint *mBonePoint;
};
#endif // EDGE_H
