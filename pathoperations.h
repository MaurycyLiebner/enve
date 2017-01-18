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
    bool isReversed() { return mReversed; }
    void setReversed(bool reversed) { mReversed = reversed; }
    bool wasAdded() { return mAdded; }
    void setAdded() { mAdded = true; }
private:
    MinimalPathPoint *mNextPoint = NULL;
    MinimalPathPoint *mPrevPoint = NULL;
    QPointF mStartCtrlPos;
    QPointF mPos;
    QPointF mEndCtrlPos;
    bool mAdded = false;
    bool mReversed = false;
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

class MinimalVectorPath;

class FullVectorPath {
public:
    FullVectorPath();

    void generateSinglePathPaths();

    void generateFromPath(const QPainterPath &path);

    int getSeparatePathsCount();

    MinimalVectorPath *getSeparatePathAt(int id);

    void intersectWith(FullVectorPath *otherPath);

    void getListOfGeneratedSeparatePaths(QList<MinimalVectorPath*> *separate, FullVectorPath *target);

    void addAllToVectorPath(VectorPath *path);

    void getSeparatePathsFromOther(FullVectorPath *other);

    const QPainterPath &getPath() { return mPath; }
private:
    QPainterPath mPath;
    QList<MinimalVectorPath*> mSeparatePaths;
};

class MinimalVectorPath {
public:
    MinimalVectorPath(FullVectorPath *parent);

    void closePath();
    void setLastPointStart(QPointF start);
    void setLastPointEnd(QPointF end);
    void setLastPointPos(QPointF pos);

    void addPoint(MinimalPathPoint *point);

    virtual ~MinimalVectorPath();

    MinimalPathPoint *getFirstPoint();

    void intersectWith(MinimalVectorPath *otherPath);

    void addAllPaths(QList<MinimalVectorPath *> *targetsList,
                     FullVectorPath *targetFull);

    void generateQPainterPath();

    bool pointInsidePath(QPointF point);

    const QPainterPath &getPath() { return mPath; }

    void addIntersectionPoint(IntersectionPathPoint *point) {
        mIntersectionPoints << point;
    }

    const QPainterPath &getParentFullPath() {
        return mParentFullPath->getPath();
    }

private:
    FullVectorPath *mParentFullPath = NULL;
    MinimalPathPoint *mFirstPoint = NULL;
    MinimalPathPoint *mLastPoint = NULL;
    QList<IntersectionPathPoint*> mIntersectionPoints;
    QPainterPath mPath;

};

class PointsBezierCubic;
#include "canvas.h"
class BezierCubic {
public:
    BezierCubic(QPointF p1, QPointF c1, QPointF c2, QPointF p2);
    BezierCubic(BezierCubic *cubic);
    virtual ~BezierCubic() {}

    QPointF getPointAtT(const qreal &t) {
        return calcCubicBezierVal(mP1, mC1, mC2, mP2, t);
    }

    qreal getTForPoint(QPointF point);
    bool intersects(BezierCubic *bezier, QPointF *intersectionPt) const;
    QRectF getPointsBoundingRect() const;

    const QPointF &getP1();
    const QPointF &getC1();
    const QPointF &getC2();
    const QPointF &getP2();
protected:
    QPointF mP1;
    QPointF mC1;
    QPointF mC2;
    QPointF mP2;
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
