#ifndef PATHANIMATOR_H
#define PATHANIMATOR_H
#include <QPainterPath>
#include "complexanimator.h"

enum CanvasMode : short;

class Edge;
class MovablePoint;

class PathPoint;

struct PathPointAnimators;

class PathAnimator : public ComplexAnimator
{
public:
    PathAnimator(BoundingBox *parentBox);
    PathAnimator();

    ~PathAnimator();

    Edge *getEgde(QPointF absPos);
    QPointF getRelCenterPosition();
    void updatePath();
    MovablePoint *getPointAt(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode);
    QPainterPath getCurrentPath() {
        return mPath;
    }

    void loadPointsFromSql(int boundingBoxId);
    void savePointsToSql(QSqlQuery *query, const int &boundingBoxId);
    PathPoint *createNewPointOnLineNear(QPointF absPos, bool adjust);
    void updateAfterFrameChanged(int currentFrame);
    qreal findPercentForPoint(QPointF point,
                              PathPoint **prevPoint,
                              qreal *error);
    void applyTransformToPoints(QMatrix transform);
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
    void replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                  PathPoint *newPoint);
    void startAllPointsTransform();
    void finishAllPointsTransform();
    void duplicatePathPointsTo(PathAnimator *target);
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

    BoundingBox *getParentBox() {
        return mParentBox;
    }

    void loadPathFromQPainterPath(const QPainterPath &path);
    void addPointToSeparatePaths(PathPoint *pointToAdd,
                                 const bool &saveUndoRedo = true);
    void setParentBox(BoundingBox *parent);
private:
    BoundingBox *mParentBox = NULL;
    QPainterPath mPath;
    QList<PathPoint*> mSeparatePaths;
    QList<PathPoint*> mPoints;
    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              PathPoint **prevPoint,
                                              PathPoint **nextPoint);
    void updatePathPointIds();
};

#endif // PATHANIMATOR_H
