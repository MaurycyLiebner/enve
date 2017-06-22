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

    ~SinglePathAnimator();

    VectorPathEdge *getEgde(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    void updatePath();
    void updateSkPath();
    MovablePoint *qra_getPointAt(const QPointF &absPtPos,
                                 const CanvasMode &currentCanvasMode,
                                 const qreal &canvasScaleInv);

    const SkPath &getCurrentSkPath() {
        return mSkPath;
    }

    //void loadPointsFromSql(int boundingBoxId);
    void savePointsToSql(QSqlQuery *query, const int &boundingBoxId);
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
    void duplicatePathPointsTo(SinglePathAnimator *target);
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

    PathAnimator *getParentPathAnimator() {
        return mParentPathAnimator;
    }

    //void loadPathFromQPainterPath(const QPainterPath &path);
    void changeAllPointsParentPathTo(SinglePathAnimator *path);
    void updatePathPointIds();
    int getChildPointIndex(PathPoint *child);

    bool SWT_isSinglePathAnimator() { return true; }
    SkPath getPathAtRelFrame(const int &relFrame);
private:
    PathAnimator *mParentPathAnimator = NULL;
    QPainterPath mPath;
    SkPath mSkPath;
    PathPoint *mFirstPoint = NULL;
    QList<QSharedPointer<PathPoint> > mPoints;
    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              PathPoint **prevPoint,
                                              PathPoint **nextPoint,
                                              const qreal &canvasScaleInv);
signals:
    void lastPointRemoved();
};

#endif // SINGLEPATHANIMATOR_H
