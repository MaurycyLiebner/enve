#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include <QLinearGradient>
#include "pathbox.h"

class PathPoint;
class BoxesGroup;
class PathAnimator;
typedef QSharedPointer<PathAnimator> PathAnimatorQSPtr;

enum CanvasMode : short;

class VectorPathEdge;

class VectorPath : public PathBox
{
public:
    VectorPath(BoxesGroup *group);
    static VectorPath *createPathFromSql(int boundingBoxId,
                                         BoxesGroup *parent);

    ~VectorPath();

    void drawSelectedSk(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const qreal &invScale);

    PathPoint *addPointAbsPos(QPointF absPtPos, PathPoint *toPoint = NULL);
    PathPoint *addPointRelPos(QPointF relPtPos, PathPoint *toPoint = NULL);

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);

    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint*> *list);

    PathPoint *addPoint(PathPoint *pointToAdd, PathPoint *toPoint);

    void removePoint(PathPoint *point);
    void replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                  PathPoint *newPoint);
    void addPointToSeparatePaths(PathPoint *pointToAdd,
                                 bool saveUndoRedo = true);
    void removePointFromSeparatePaths(PathPoint *pointToRemove,
                                      bool saveUndoRedo = true);
    void appendToPointsList(PathPoint *point,
                            bool saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point,
                              bool saveUndoRedo = true);

    PathPoint *addPointRelPos(QPointF relPos,
                              QPointF startRelPos, QPointF endRelPos,
                              PathPoint *toPoint = NULL);
    int prp_saveToSql(QSqlQuery *query, const int &parentId);

    PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
    qreal percentAtPoint(QPointF absPos, qreal distTolerance,
                         qreal maxPercent, qreal minPercent,
                         bool *found = NULL, QPointF *posInPath = NULL);

    void showContextMenu(QPoint globalPos);

    void deletePointAndApproximate(PathPoint *pointToRemove);
    virtual void prp_loadFromSql(const int &boundingBoxId);

    void applyTransformToPoints(QMatrix transform);
    void applyCurrentTransformation();
    VectorPathEdge *getEgde(const QPointF &absPos,
                            const qreal &canvasScaleInv);

    void loadPathFromQPainterPath(const QPainterPath &path);
    void loadPathFromSkPath(const SkPath &path);

    void duplicatePathPointsTo(VectorPath *target);
//    void disconnectPoints(PathPoint *point1, PathPoint *point2);
//    void connectPoints(PathPoint *point1, PathPoint *point2);
    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    PathAnimator *getPathAnimator();
    void duplicatePathAnimatorFrom(PathAnimator *source);
    void removeChildPathAnimator(PathAnimator *path);
    void updatePath();

    bool SWT_isVectorPath() { return true; }
protected:
    void updatePathPointIds();
    PathAnimatorQSPtr mPathAnimator;

    void loadPointsFromSql(int boundingBoxId);

    qreal findPercentForPoint(QPointF point,
                              PathPoint **prevPoint,
                              qreal *error);
    QPointF getRelCenterPosition();

    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              PathPoint **prevPoint,
                                              PathPoint **nextPoint);
};

#endif // VECTORPATH_H
