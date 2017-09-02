#ifndef SINGLEVECTORPATHANIMATOR_H
#define SINGLEVECTORPATHANIMATOR_H


#include <QPainterPath>
#include "Animators/PathAnimators/singlepathanimator.h"

enum CanvasMode : short;

class PathAnimator;
class VectorPathEdge;
class MovablePoint;
class NodePoint;
class Canvas;
#include "skiaincludes.h"

class SingleVectorPathAnimator : public SinglePathAnimator {
    Q_OBJECT
public:
    SingleVectorPathAnimator(PathAnimator *parentPath);

    VectorPathEdge *getEdge(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                 const CanvasMode &currentCanvasMode,
                                 const qreal &canvasScaleInv);

    NodePoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);

    void applyTransformToPoints(const QMatrix &transform);
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
    void startAllPointsTransform();
    void finishAllPointsTransform();
    void duplicateNodePointsTo(SingleVectorPathAnimator *target);
    NodePoint *addPoint(NodePoint *pointToAdd, NodePoint *toPoint);
    NodePoint *addPointAbsPos(const QPointF &absPtPos, NodePoint *toPoint);
    void deletePointAndApproximate(NodePoint *pointToRemove);

    void drawSelected(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const qreal &invScale,
                      const SkMatrix &combinedTransform);

    void selectAndAddContainedPointsToList(
            const QRectF &absRect, QList<MovablePoint *> *list);

    //void loadPathFromQPainterPath(const QPainterPath &path);
    void changeAllPointsParentPathTo(SingleVectorPathAnimator *path);
    void updateNodePointIds();
    int getChildPointIndex(NodePoint *child);

    SkPath getPathAtRelFrame(const int &relFrame);

    SingleVectorPathAnimator *makeDuplicate() {
        SingleVectorPathAnimator *path =
                new SingleVectorPathAnimator(mParentPathAnimator);
        duplicateNodePointsTo(path);
        return path;
    }

    void setParentPath(PathAnimator *parentPath) {
        mParentPathAnimator = parentPath;
    }

    PathAnimator *getParentPathAnimator() {
        return mParentPathAnimator;
    }
    void selectAllPoints(Canvas *canvas);

    bool closedPath() const {
        return false;
    }
private:
    PathAnimator *mParentPathAnimator = NULL;
    NodePoint *mFirstPoint = NULL;
    QList<QSharedPointer<NodePoint> > mPoints;

    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              NodePoint **prevPoint,
                                              NodePoint **nextPoint,
                                              const qreal &canvasScaleInv);
};

#endif // SINGLEVECTORPATHANIMATOR_H
