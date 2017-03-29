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
    MovablePoint *qra_getPointAt(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode);
    QPainterPath getCurrentPath() {
        return mPath;
    }

    void prp_loadFromSql(const int &boundingBoxId);
    int prp_saveToSql(QSqlQuery *query,
                      const int &boundingBoxId);
    void prp_makeDuplicate(Property *property) {
        duplicatePathsTo((PathAnimator*)property);
    }

    Property *prp_makeDuplicate() {
        PathAnimator *newAnimator = new PathAnimator();
        prp_makeDuplicate(newAnimator);
        return newAnimator;
    }


    PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust);
    void updateAfterFrameChanged(const int &currentFrame);
    qreal findPercentForPoint(const QPointF &point,
                              PathPoint **prevPoint,
                              qreal *error);
    void applyTransformToPoints(const QMatrix &transform);

    void startAllPointsTransform();
    void finishAllPointsTransform();
    void duplicatePathsTo(PathAnimator *target);

    void drawSelected(QPainter *p,
                      const CanvasMode &currentCanvasMode,
                      const QMatrix &combinedTransform);
    void selectAndAddContainedPointsToList(
            const QRectF &absRect, QList<MovablePoint *> *list);

    BoundingBox *getParentBox() {
        return mParentBox;
    }

    void loadPathFromQPainterPath(const QPainterPath &path);

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
