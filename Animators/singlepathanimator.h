#ifndef SINGLEPATHANIMATOR_H
#define SINGLEPATHANIMATOR_H

#include <QPainterPath>
#include "complexanimator.h"

enum CanvasMode : short;

class PathAnimator;
class Edge;
class MovablePoint;

class PathPoint;

struct PathPointAnimators;

class SinglePathAnimator : public ComplexAnimator
{
    Q_OBJECT
public:
    SinglePathAnimator(PathAnimator *parentPath);

    ~SinglePathAnimator();

    Edge *getEgde(QPointF absPos);
    QPointF getRelCenterPosition();
    void updatePath();
    MovablePoint *qra_getPointAt(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode);
    QPainterPath getCurrentPath() {
        return mPath;
    }

    void loadPointsFromSql(int boundingBoxId);
    void savePointsToSql(QSqlQuery *query, const int &boundingBoxId);
    PathPoint *createNewPointOnLineNear(QPointF absPos, bool adjust);
    void updateAfterFrameChanged(int currentFrame);
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
    void drawSelected(QPainter *p,
                      const CanvasMode &currentCanvasMode,
                      const QMatrix &combinedTransform);
    void selectAndAddContainedPointsToList(
            const QRectF &absRect, QList<MovablePoint *> *list);

    PathAnimator *getParentPathAnimator() {
        return mParentPathAnimator;
    }

    //void loadPathFromQPainterPath(const QPainterPath &path);
    void changeAllPointsParentPathTo(SinglePathAnimator *path);
    void updatePathPointIds();
private:
    PathAnimator *mParentPathAnimator = NULL;
    QPainterPath mPath;
    PathPoint *mFirstPoint = NULL;
    QList<PathPoint*> mPoints;
    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              PathPoint **prevPoint,
                                              PathPoint **nextPoint);
signals:
    void lastPointRemoved();
};

#endif // SINGLEPATHANIMATOR_H
