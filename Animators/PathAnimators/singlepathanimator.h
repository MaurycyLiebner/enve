#ifndef SINGLEPATHANIMATOR_H
#define SINGLEPATHANIMATOR_H

#include <QPainterPath>
#include "Animators/complexanimator.h"

enum CanvasMode : short;

class VectorPathEdge;
class MovablePoint;
class PathPoint;
#include "skiaincludes.h"

class SinglePathAnimator : public ComplexAnimator {
    Q_OBJECT
public:
    SinglePathAnimator();

    virtual VectorPathEdge *getEgde(const QPointF &,
                                    const qreal &) { return NULL; }
    virtual MovablePoint *getPointAtAbsPos(const QPointF &,
                                         const CanvasMode &,
                                         const qreal &) { return NULL; }
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
                              const SkMatrix &combinedTransform) = 0;

    virtual void selectAndAddContainedPointsToList(
                    const QRectF &,
                    QList<MovablePoint *> *) {}

    //void loadPathFromQPainterPath(const QPainterPath &path);

    bool SWT_isSinglePathAnimator() { return true; }
    virtual SkPath getPathAtRelFrame(const int &relFrame) = 0;

    virtual SinglePathAnimator *makeDuplicate() = 0;
protected:
};

#endif // SINGLEPATHANIMATOR_H
