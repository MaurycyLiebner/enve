#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "pathpoint.h"
#include <QLinearGradient>
#include "Animators/pathanimator.h"
#include "Boxes/pathbox.h"

class BoxesGroup;

class MainWindow;

class PathPivot;

enum CanvasMode : short;

class Edge;

class VectorShapesMenu;

class VectorPath : public PathBox
{
public:
    VectorPath(BoxesGroup *group);
    static VectorPath *createPathFromSql(int boundingBoxId, BoxesGroup *parent);

    ~VectorPath();

    void drawSelected(QPainter *p, const CanvasMode &currentCanvasMode);

    PathPoint *addPointAbsPos(QPointF absPtPos, PathPoint *toPoint = NULL);
    PathPoint *addPointRelPos(QPointF relPtPos, PathPoint *toPoint = NULL);

    MovablePoint *getPointAt(const QPointF &absPtPos, const CanvasMode &currentCanvasMode);

    void selectAndAddContainedPointsToList(QRectF absRect, QList<MovablePoint*> *list);

    PathPoint *addPoint(PathPoint *pointToAdd, PathPoint *toPoint);

    void removePoint(PathPoint *point);
    void replaceSeparatePathPoint(PathPoint *pointBeingReplaced, PathPoint *newPoint);
    void addPointToSeparatePaths(PathPoint *pointToAdd, bool saveUndoRedo = true);
    void removePointFromSeparatePaths(PathPoint *pointToRemove, bool saveUndoRedo = true);
    void appendToPointsList(PathPoint *point, bool saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point, bool saveUndoRedo = true);

    PathPoint *addPointRelPos(QPointF relPos,
                              QPointF startRelPos, QPointF endRelPos,
                              PathPoint *toPoint = NULL);
    int saveToSql(QSqlQuery *query, int parentId);

    PathPoint *createNewPointOnLineNear(QPointF absPos, bool adjust);
    qreal percentAtPoint(QPointF absPos, qreal distTolerance,
                         qreal maxPercent, qreal minPercent,
                         bool *found = NULL, QPointF *posInPath = NULL);

    void startAllPointsTransform() {
        //mPathAnimator.startAllPointsTransform();
    }

    void finishAllPointsTransform() {
        //mPathAnimator.finishAllPointsTransform();
    }

    void showContextMenu(QPoint globalPos);

    void deletePointAndApproximate(PathPoint *pointToRemove);
    virtual void loadFromSql(int boundingBoxId);

    void applyTransformToPoints(QMatrix transform);
    void applyCurrentTransformation();
    Edge *getEgde(QPointF absPos);

    void loadPathFromQPainterPath(const QPainterPath &path);

    void duplicatePathPointsTo(VectorPath *target);
//    void disconnectPoints(PathPoint *point1, PathPoint *point2);
//    void connectPoints(PathPoint *point1, PathPoint *point2);
    void makeDuplicate(BoundingBox *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    PathAnimator *getPathAnimator();
    void duplicatePathAnimatorFrom(PathAnimator *source);
    void removeChildPathAnimator(PathAnimator *path);
    void updatePath();
protected:
    void updatePathPointIds();
    PathAnimator mPathAnimator;

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
