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

    bool isIntersection();

    void setSibling(IntersectionPathPoint *sibling);
    IntersectionPathPoint *getSibling();
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

    void removeNonIntersectionPointsContainedInAndSaveIntersectionPoints(
            MinimalVectorPath *path);

    void generateQPainterPath();

    bool pointInsidePath(QPointF point);

    const QPainterPath &getPath() { return mPath; }
private:
    MinimalPathPoint *mFirstPoint = NULL;
    MinimalPathPoint *mLastPoint = NULL;
    QList<IntersectionPathPoint*> mIntersectionPoints;
    QList<MinimalPathPoint*> mPoints;
    QPainterPath mPath;

};

class FullVectorPath {
public:
    FullVectorPath() {}

    void generateSignlePathPaths() {
        foreach(MinimalVectorPath *separatePath, mSeparatePaths) {
            separatePath->generateQPainterPath();
        }
    }

    void generateFromPath(const QPainterPath &path);

    int getSeparatePathsCount() {
        return mSeparatePaths.count();
    }

    MinimalVectorPath *getSeparatePathAt(int id) {
        return mSeparatePaths.at(id);
    }

    void intersectWith(FullVectorPath *otherPath) {
        int otherCount = otherPath->getSeparatePathsCount();
        for(int i = 0; i < otherCount; i++) {
            MinimalVectorPath *otherSPath = otherPath->getSeparatePathAt(i);
            foreach(MinimalVectorPath *thisSPath, mSeparatePaths) {
                thisSPath->intersectWith(otherSPath);
            }
        }
    }

    void getListOfGeneratedSeparatePaths(QList<MinimalVectorPath*> *separate) {
        foreach(MinimalVectorPath *thisSPath, mSeparatePaths) {
            thisSPath->addAllPaths(separate);
        }
    }

    void addAllToVectorPath(VectorPath *path) {
        foreach(MinimalVectorPath *separatePath, mSeparatePaths) {
            MinimalPathPoint *firstPoint = separatePath->getFirstPoint();
            MinimalPathPoint *point = firstPoint;
            PathPoint *firstPathPoint = NULL;
            PathPoint *lastPathPoint = NULL;
            do {
                lastPathPoint = path->addPointRelPos(point->getPos(),
                                     point->getStartPos(),
                                     point->getEndPos(),
                                     lastPathPoint);
                if(firstPathPoint == NULL) {
                    firstPathPoint = lastPathPoint;
                }
                point = point->getNextPoint();
            } while(point != firstPoint);
            lastPathPoint->connectToPoint(firstPathPoint);
        }
    }

    void getSeparatePathsFromOther(FullVectorPath *other) {
        other->getListOfGeneratedSeparatePaths(&mSeparatePaths);
    }

private:
    QList<MinimalVectorPath*> mSeparatePaths;
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
