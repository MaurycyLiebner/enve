#ifndef SINGLEPATHANIMATOR_H
#define SINGLEPATHANIMATOR_H

#include <QPainterPath>
#include "Animators/complexanimator.h"

enum CanvasMode : short;

class VectorPathEdge;
class MovablePoint;
class NodePoint;
#include "skiaincludes.h"

class SinglePathAnimator : public ComplexAnimator {
    Q_OBJECT
public:
    SinglePathAnimator();

    virtual VectorPathEdge *getEdge(const QPointF &,
                                    const qreal &) { return NULL; }
    virtual MovablePoint *getPointAtAbsPos(const QPointF &,
                                         const CanvasMode &,
                                         const qreal &) { return NULL; }
    virtual NodePoint *createNewPointOnLineNear(const QPointF &,
                                                const bool &,
                                                const qreal &) { return NULL; }

    virtual void applyTransformToPoints(const QMatrix &) {}
    void disconnectPoints(NodePoint *point1, NodePoint *point2);
    void connectPoints(NodePoint *point1, NodePoint *point2);
    NodePoint *addPointRelPos(const QPointF &relPos,
                              const QPointF &startRelPos,
                              const QPointF &endRelPos,
                              NodePoint *toPoint);
    NodePoint *addPointRelPos(const QPointF &relPtPos,
                              NodePoint *toPoint);
    void appendToPointsList(NodePoint *point,
                            const bool &saveUndoRedo = true);
    void removeFromPointsList(NodePoint *point,
                              const bool &saveUndoRedo = true);
    void removePoint(NodePoint *point);
    void replaceSeparateNodePoint(NodePoint *newPoint,
                                  const bool &saveUndoRedo = true);
    virtual void startAllPointsTransform() {}
    virtual void finishAllPointsTransform() {}
    void duplicateNodePointsTo(SinglePathAnimator *target);

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
