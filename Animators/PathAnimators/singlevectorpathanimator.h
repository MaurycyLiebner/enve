#ifndef SINGLEVECTORPATHANIMATOR_H
#define SINGLEVECTORPATHANIMATOR_H


#include <QPainterPath>
#include "Animators/PathAnimators/singlepathanimator.h"

enum CanvasMode : short;

class PathAnimator;
class VectorPathEdge;
class MovablePoint;
class PathPoint;
#include "skiaincludes.h"

class SingleVectorPathAnimator : public SinglePathAnimator {
    Q_OBJECT
public:
    SingleVectorPathAnimator(PathAnimator *parentPath);

    VectorPathEdge *getEgde(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
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

    SingleVectorPathAnimator *makeDuplicate() {
        SingleVectorPathAnimator *path =
                new SingleVectorPathAnimator(mParentPathAnimator);
        duplicatePathPointsTo(path);
        return path;
    }

    void setParentPath(PathAnimator *parentPath) {
        mParentPathAnimator = parentPath;
    }

    PathAnimator *getParentPathAnimator() {
        return mParentPathAnimator;
    }
private:
    PathAnimator *mParentPathAnimator = NULL;
    PathPoint *mFirstPoint = NULL;
    QList<QSharedPointer<PathPoint> > mPoints;

    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              PathPoint **prevPoint,
                                              PathPoint **nextPoint,
                                              const qreal &canvasScaleInv);
};

#endif // SINGLEVECTORPATHANIMATOR_H
