#ifndef SINGLEPATHANIMATOR_H
#define SINGLEPATHANIMATOR_H

#include <QPainterPath>
#include "complexanimator.h"

enum CanvasMode : short;

class PathAnimator;
class VectorPathEdge;
class MovablePoint;
class PathPoint;
#include "skiaincludes.h"

class SinglePathAnimator : public ComplexAnimator {
    Q_OBJECT
public:
    SinglePathAnimator(PathAnimator *parentPath);

    virtual VectorPathEdge *getEgde(const QPointF &,
                                    const qreal &) { return NULL; }
    virtual void updatePath() = 0;
    virtual void updateSkPath() = 0;
    virtual MovablePoint *qra_getPointAt(const QPointF &,
                                         const CanvasMode &,
                                         const qreal &) { return NULL; }

    const SkPath &getCurrentSkPath() {
        return mSkPath;
    }

    virtual void saveToSql(QSqlQuery *query,
                           const int &boundingBoxId) = 0;

    //void loadPointsFromSql(int boundingBoxId);
    virtual PathPoint *createNewPointOnLineNear(const QPointF &,
                                                const bool &,
                                                const qreal &) { return NULL; }
    qreal findPercentForPoint(const QPointF &point,
                              PathPoint **prevPoint,
                              qreal *error);
    virtual void applyTransformToPoints(const QMatrix &) {}
    void disconnectPoints(PathPoint *point1, PathPoint *point2);
    void connectPoints(PathPoint *point1, PathPoint *point2);
    PathPoint *addPointRelPos(const QPointF &relPos,
                              const QPointF &startRelPos,
                              const QPointF &endRelPos,
                              PathPoint *toPoint);
    PathPoint *addPointRelPos(const QPointF &relPtPos,
                              PathPoint *toPoint);
    void appendToPointsList(PathPoint *point,
                            const bool &saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point,
                              const bool &saveUndoRedo = true);
    void removePoint(PathPoint *point);
    void replaceSeparatePathPoint(PathPoint *newPoint,
                                  const bool &saveUndoRedo = true);
    virtual void startAllPointsTransform() {}
    virtual void finishAllPointsTransform() {}
    void duplicatePathPointsTo(SinglePathAnimator *target);

    virtual void drawSelected(SkCanvas *canvas,
                              const CanvasMode &,
                              const qreal &invScale,
                              const SkMatrix &combinedTransform);

    virtual void selectAndAddContainedPointsToList(
                    const QRectF &,
                    QList<MovablePoint *> *) {}

    PathAnimator *getParentPathAnimator() {
        return mParentPathAnimator;
    }

    //void loadPathFromQPainterPath(const QPainterPath &path);

    bool SWT_isSinglePathAnimator() { return true; }
    virtual SkPath getPathAtRelFrame(const int &relFrame) = 0;

    virtual SinglePathAnimator *makeDuplicate(PathAnimator *parentPath) = 0;
protected:
    PathAnimator *mParentPathAnimator = NULL;
    QPainterPath mPath;
    SkPath mSkPath;
};

class SingleVectorPathAnimator : public SinglePathAnimator {
    Q_OBJECT
public:
    SingleVectorPathAnimator(PathAnimator *parentPath);

    VectorPathEdge *getEgde(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    void updatePath();
    void updateSkPath();
    MovablePoint *qra_getPointAt(const QPointF &absPtPos,
                                 const CanvasMode &currentCanvasMode,
                                 const qreal &canvasScaleInv);


    //void loadPointsFromSql(int boundingBoxId);
    void saveToSql(QSqlQuery *query,
                   const int &boundingBoxId);
    PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
    qreal findPercentForPoint(const QPointF &point,
                              PathPoint **prevPoint,
                              qreal *error);
    void applyTransformToPoints(const QMatrix &transform);
    void disconnectPoints(PathPoint *point1, PathPoint *point2);
    void connectPoints(PathPoint *point1, PathPoint *point2);
    PathPoint *addPointRelPos(const QPointF &relPos,
                              const QPointF &startRelPos,
                              const QPointF &endRelPos,
                              PathPoint *toPoint);
    PathPoint *addPointRelPos(const QPointF &relPtPos,
                              PathPoint *toPoint);
    void appendToPointsList(PathPoint *point,
                            const bool &saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point,
                              const bool &saveUndoRedo = true);
    void removePoint(PathPoint *point);
    void replaceSeparatePathPoint(PathPoint *newPoint,
                                  const bool &saveUndoRedo = true);
    void startAllPointsTransform();
    void finishAllPointsTransform();
    void duplicatePathPointsTo(SingleVectorPathAnimator *target);
    void removePointFromSeparatePaths(PathPoint *pointToRemove,
                                      bool saveUndoRedo = true);
    PathPoint *addPoint(PathPoint *pointToAdd, PathPoint *toPoint);
    PathPoint *addPointAbsPos(const QPointF &absPtPos, PathPoint *toPoint);
    void deletePointAndApproximate(PathPoint *pointToRemove);

    void drawSelected(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const qreal &invScale,
                      const SkMatrix &combinedTransform);

    void selectAndAddContainedPointsToList(
            const QRectF &absRect, QList<MovablePoint *> *list);

    //void loadPathFromQPainterPath(const QPainterPath &path);
    void changeAllPointsParentPathTo(SingleVectorPathAnimator *path);
    void updatePathPointIds();
    int getChildPointIndex(PathPoint *child);

    SkPath getPathAtRelFrame(const int &relFrame);

    SinglePathAnimator *makeDuplicate(PathAnimator *parentPath) {
        SingleVectorPathAnimator *path =
                new SingleVectorPathAnimator(parentPath);
        duplicatePathPointsTo(path);
        return path;
    }

private:
    PathPoint *mFirstPoint = NULL;
    QList<QSharedPointer<PathPoint> > mPoints;

    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              PathPoint **prevPoint,
                                              PathPoint **nextPoint,
                                              const qreal &canvasScaleInv);
};

#endif // SINGLEPATHANIMATOR_H
