#ifndef PATHOPERATIONS_H
#define PATHOPERATIONS_H
#include <QPainterPath>
#include "Boxes/vectorpath.h"
#include "nodepoint.h"

class MinimalNodePoint {
public:
    MinimalNodePoint();
    MinimalNodePoint(QPointF ctrlStart, QPointF pos, QPointF ctrlEnd);
    virtual ~MinimalNodePoint() {}

    void setNextPoint(MinimalNodePoint *point);
    void setPrevPoint(MinimalNodePoint *point);

    MinimalNodePoint *getNextPoint();
    MinimalNodePoint *getPrevPoint();

    void setStartCtrlPos(const QPointF &pos);
    void setEndCtrlPos(const QPointF &pos);
    void setPos(const QPointF &pos);

    QPointF getStartPos();
    QPointF getEndPos();
    QPointF getPos();

    virtual bool isIntersection();
    bool isReversed() { return mReversed; }
    void setReversed(bool reversed) { mReversed = reversed; }
    bool wasAdded() { return mAdded; }
    void setAdded() { mAdded = true; }
private:
    MinimalNodePoint *mNextPoint = NULL;
    MinimalNodePoint *mPrevPoint = NULL;
    QPointF mStartCtrlPos;
    QPointF mPos;
    QPointF mEndCtrlPos;
    bool mAdded = false;
    bool mReversed = false;
};

class IntersectionNodePoint : public MinimalNodePoint
{
public:
    IntersectionNodePoint();
    IntersectionNodePoint(QPointF start, QPointF pos, QPointF end);
    ~IntersectionNodePoint();

    bool isIntersection();

    void setSibling(IntersectionNodePoint *sibling);
    IntersectionNodePoint *getSibling();

    void fixSiblingSideCtrlPoint();

private:
    IntersectionNodePoint *mSiblingIntPoint;
};

class MinimalVectorPath;

class FullVectorPath {
public:
    FullVectorPath();

    void generateSinglePathPaths();

    //void generateFromPath(const QPainterPath &path);
    void generateFromPath(const SkPath &path);

    int getSeparatePathsCount();

    MinimalVectorPath *getSeparatePathAt(int id);

    void intersectWith(FullVectorPath *otherPath,
                       const bool &unionInterThis,
                       const bool &unionInterOther);

    void getListOfGeneratedSeparatePaths(QList<MinimalVectorPath*> *separate, FullVectorPath *target);

    void addAllToVectorPath(PathAnimator *path);

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

    void addPoint(MinimalNodePoint *point);

    virtual ~MinimalVectorPath();

    MinimalNodePoint *getFirstPoint();

    void intersectWith(MinimalVectorPath *otherPath,
                       const bool &unionInterThis,
                       const bool &unionInterOther);

    void addAllPaths(QList<MinimalVectorPath *> *targetsList,
                     FullVectorPath *targetFull);

    void generateQPainterPath();

    bool pointInsidePath(QPointF point);

    const QPainterPath &getPath() { return mPath; }

    void addIntersectionPoint(IntersectionNodePoint *point) {
        mIntersectionPoints << point;
    }

    const QPainterPath &getParentFullPath() {
        return mParentFullPath->getPath();
    }

private:
    FullVectorPath *mParentFullPath = NULL;
    MinimalNodePoint *mFirstPoint = NULL;
    MinimalNodePoint *mLastPoint = NULL;
    QList<IntersectionNodePoint*> mIntersectionPoints;
    QPainterPath mPath;

};

class PointsBezierCubic;
#include "canvas.h"
class BezierCubic {
public:
    BezierCubic(const QPointF &p1, const QPointF &c1,
                const QPointF &c2, const QPointF &p2);
    BezierCubic(BezierCubic *cubic);
    virtual ~BezierCubic() {}

    QPointF getPointAtT(const qreal &t);

    qreal getTForPoint(QPointF point);

    QRectF getPointsBoundingRect() const;


    const QPointF &getP1() const { return mP1; }

    const QPointF &getC1() const { return mC1; }

    const QPointF &getC2() const { return mC2; }

    const QPointF &getP2() const { return mP2; }
protected:
    QPointF mP1;
    QPointF mC1;
    QPointF mC2;
    QPointF mP2;
};

class PointsBezierCubic : public BezierCubic {
public:
    PointsBezierCubic(MinimalNodePoint *mpp1,
                      MinimalNodePoint *mpp2,
                      MinimalVectorPath *parentPath);

    void setPoints(MinimalNodePoint *mpp1, MinimalNodePoint *mpp2);

    PointsBezierCubic *intersectWith(PointsBezierCubic *bezier);

    IntersectionNodePoint *addIntersectionPointAt(QPointF pos);

    void setNextCubic(PointsBezierCubic *cubic);
    void setPrevCubic(PointsBezierCubic *cubic);

    PointsBezierCubic *getNextCubic();
    PointsBezierCubic *getPrevCubic();

    IntersectionNodePoint *divideCubicAtPointAndReturnIntersection(
                                const QPointF &pos);

    void disconnect();
    bool intersects(PointsBezierCubic *bezier,
                    QList<QPointF> *intersectionPts,
                    QList<int> *bezierIntersectionPts) const;
    MinimalNodePoint *getMPP1() { return mMPP1; }
    MinimalNodePoint *getMPP2() { return mMPP2; }
private:
    PointsBezierCubic *mNextCubic = NULL;
    PointsBezierCubic *mPrevCubic = NULL;
    MinimalNodePoint *mMPP1;
    MinimalNodePoint *mMPP2;
    MinimalVectorPath *mParentPath = NULL;
};

#endif // PATHOPERATIONS_H
