#ifndef PATHOPERATIONS_H
#define PATHOPERATIONS_H
#include <QPainterPath>
#include "Boxes/vectorpath.h"
#include "pathpoint.h"

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
    bool wasAdded() { return mAdded; }
    void setAdded() { mAdded = true; }
    bool hasNoConnections();
private:
    MinimalPathPoint *mNextPoint = NULL;
    MinimalPathPoint *mPrevPoint = NULL;
    QPointF mStartCtrlPos;
    QPointF mPos;
    QPointF mEndCtrlPos;
    bool mAdded = false;
};

class IntersectionPathPoint : public MinimalPathPoint
{
public:
    IntersectionPathPoint();
    IntersectionPathPoint(QPointF start, QPointF pos, QPointF end);
    ~IntersectionPathPoint();

    bool isIntersection();

    void setSibling(IntersectionPathPoint *sibling);
    IntersectionPathPoint *getSibling();

    void fixSiblingSideCtrlPoint();

private:
    IntersectionPathPoint *mSiblingIntPoint;
};

class MinimalVectorPath {
public:
    MinimalVectorPath();

    void closePath();
    void setLastPointStart(QPointF start);
    void setLastPointEnd(QPointF end);
    void setLastPointPos(QPointF pos);

    void addPoint(MinimalPathPoint *point);

    virtual ~MinimalVectorPath();

    MinimalPathPoint *getFirstPoint();

    void intersectWith(MinimalVectorPath *otherPath);

    void addAllPaths(QList<MinimalVectorPath *> *targetsList);

    void generateQPainterPath();

    bool pointInsidePath(QPointF point);

    const QPainterPath &getPath() { return mPath; }

    void addIntersectionPoint(IntersectionPathPoint *point) {
        mIntersectionPoints << point;
    }

private:
    MinimalPathPoint *mFirstPoint = NULL;
    MinimalPathPoint *mLastPoint = NULL;
    QList<IntersectionPathPoint*> mIntersectionPoints;
    QPainterPath mPath;

};

class FullVectorPath {
public:
    FullVectorPath();

    void generateSignlePathPaths();

    void generateFromPath(const QPainterPath &path);

    int getSeparatePathsCount();

    MinimalVectorPath *getSeparatePathAt(int id);

    void intersectWith(FullVectorPath *otherPath);

    void getListOfGeneratedSeparatePaths(QList<MinimalVectorPath*> *separate);

    void addAllToVectorPath(VectorPath *path);

    void getSeparatePathsFromOther(FullVectorPath *other);

private:
    QList<MinimalVectorPath*> mSeparatePaths;
};

class PointsBezierCubic;
#include "canvas.h"
class BezierCubic {
public:
    BezierCubic(QPointF p1, QPointF c1, QPointF c2, QPointF p2);
    BezierCubic(BezierCubic *cubic);
    virtual ~BezierCubic() {}

    qreal getTForPoint(QPointF point);
    void generatePath();
    const QPainterPath &getAsPainterPath();
    bool intersects(BezierCubic *bezier) const;
    QRectF getPointsBoundingRect() const;
    bool intersectWithSub(PointsBezierCubic *otherBezier,
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
                      MinimalPathPoint *mpp2,
                      MinimalVectorPath *parentPath);

    void setPoints(MinimalPathPoint *mpp1, MinimalPathPoint *mpp2);

    void intersectWith(PointsBezierCubic *bezier);

    IntersectionPathPoint *addIntersectionPointAt(QPointF pos);

    void setNextCubic(PointsBezierCubic *cubic);
    void setPrevCubic(PointsBezierCubic *cubic);

    PointsBezierCubic *getNextCubic();
    PointsBezierCubic *getPrevCubic();

    IntersectionPathPoint *divideCubicAtPointAndReturnIntersection(
                                const QPointF &pos);

    void disconnect();

private:
    PointsBezierCubic *mNextCubic = NULL;
    PointsBezierCubic *mPrevCubic = NULL;
    MinimalPathPoint *mMPP1;
    MinimalPathPoint *mMPP2;
    MinimalVectorPath *mParentPath = NULL;
};

#endif // PATHOPERATIONS_H
