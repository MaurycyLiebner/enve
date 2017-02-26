#ifndef PATHANIMATOR_H
#define PATHANIMATOR_H
#include <QPainterPath>
#include "complexanimator.h"

enum CanvasMode : short;

class Edge;
class MovablePoint;

class PathPoint;
class SinglePathAnimator;

struct PathPointAnimators;

class PathAnimator : public ComplexAnimator
{
    Q_OBJECT
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
    PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust);
    void updateAfterFrameChanged(const int &currentFrame);
    qreal findPercentForPoint(const QPointF &point,
                              PathPoint **prevPoint,
                              qreal *error);
    void applyTransformToPoints(const QMatrix &transform);
    void disconnectPoints(PathPoint *point1, PathPoint *point2);
    void connectPoints(PathPoint *point1, PathPoint *point2);
    void appendToPointsList(PathPoint *point,
                            const bool &saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point,
                              const bool &saveUndoRedo = true);
    void removePoint(PathPoint *point);
    void replaceSeparatePathPoint(PathPoint *pointBeingReplaced,
                                  PathPoint *newPoint);
    void startAllPointsTransform();
    void finishAllPointsTransform();
    void duplicatePathsTo(PathAnimator *target);
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
    void addSinglePathAnimator(SinglePathAnimator *path,
                               const bool &saveUndoRedo = true);
    void removeSinglePathAnimator(SinglePathAnimator *path,
                                  const bool &saveUndoRedo = true);
private:
    BoundingBox *mParentBox = NULL;
    QPainterPath mPath;
    QList<SinglePathAnimator*> mSinglePaths;
signals:
    void lastSinglePathRemoved();
};

#endif // PATHANIMATOR_H
