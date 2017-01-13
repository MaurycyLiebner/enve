#ifndef PATHOPERATIONS_H
#define PATHOPERATIONS_H
#include <QPainterPath>

class MinimalPathPoint {
public:
    MinimalPathPoint();
    MinimalPathPoint(QPointF ctrlStart, QPointF pos, QPointF ctrlEnd);
    MinimalPathPoint(MinimalPathPoint *point);
    virtual ~MinimalPathPoint() {}

    void setNextPoint(MinimalPathPoint *point);
    void setPrevPoint(MinimalPathPoint *point);

    MinimalPathPoint *getNextPoint();
    MinimalPathPoint *getPrevPoint();

    void setStartCtrlPos(QPointF pos);
    void setEndCtrlPos(QPointF pos);
    void setPos(QPointF pos);

    QPointF getStartPos();
    QPointF getEndPos();
    QPointF getPos();

    virtual bool isIntersection();
private:
    MinimalPathPoint *mNextPoint = NULL;
    MinimalPathPoint *mPrevPoint = NULL;
    QPointF mStartCtrlPos;
    QPointF mPos;
    QPointF mEndCtrlPos;
};

class IntersectionPathPoint : public MinimalPathPoint
{
public:
    IntersectionPathPoint();
    IntersectionPathPoint(QPointF start, QPointF pos, QPointF end);

    bool isIntersection();

    void setSibling(IntersectionPathPoint *sibling);
    IntersectionPathPoint *getSibling();
private:
    IntersectionPathPoint *mSiblingIntPoint;
};

class MinimalVectorPath {
public:
    MinimalVectorPath();

    void setLastPointStart(QPointF start);
    void setLastPointEnd(QPointF end);
    void setLastPointPos(QPointF pos);

    void addPoint(MinimalPathPoint *point);

    virtual ~MinimalVectorPath();

    MinimalPathPoint *getFirstPoint();

    void intersectWith(MinimalVectorPath *otherPath);

    void addAllPointsToPath(MinimalVectorPath *target);

    void removeNonIntersectionPointsContainedInAndSaveIntersectionPoints(
            MinimalVectorPath *path);

    void generateQPainterPath();

    bool pointInsidePath(QPointF point);
private:
    MinimalPathPoint *mFirstPoint = NULL;
    MinimalPathPoint *mLastPoint = NULL;
    QList<IntersectionPathPoint*> mIntersectionPoints;
    QList<MinimalPathPoint*> mPoints;
    QPainterPath mPath;

};

class PointsBezierCubic;
#include "canvas.h"
class BezierCubic {
public:
    BezierCubic(QPointF p1, QPointF c1, QPointF c2, QPointF p2);
    BezierCubic(BezierCubic *cubic);

    qreal getTForPoint(QPointF point);
    void generatePath();
    const QPainterPath &getAsPainterPath();
    bool intersects(BezierCubic *bezier) const;
    QRectF getPointsBoundingRect() const;
    void intersectWithSub(PointsBezierCubic *otherBezier,
                          PointsBezierCubic *parentBezier) const;
    qreal percentAtLength(qreal length);

    const QPointF &getP1();
    const QPointF &getC1();
    const QPointF &getC2();
    const QPointF &getP2();

protected:
    QPointF mP1;
    QPointF mC1;
    QPointF mC2;
    QPointF mP2;
    QPainterPath mPainterPath;
};

class PointsBezierCubic : public BezierCubic {
public:
    PointsBezierCubic(MinimalPathPoint *mpp1,
                      MinimalPathPoint *mpp2);

    void intersectWith(PointsBezierCubic *bezier);

    IntersectionPathPoint *addIntersectionPointAt(QPointF pos);
private:
    MinimalPathPoint *mMPP1;
    MinimalPathPoint *mMPP2;
};

#endif // PATHOPERATIONS_H
