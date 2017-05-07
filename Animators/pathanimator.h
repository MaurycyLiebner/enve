#ifndef PATHANIMATOR_H
#define PATHANIMATOR_H
#include <QPainterPath>
#include "complexanimator.h"

enum CanvasMode : short;

class VectorPathEdge;
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

    VectorPathEdge *getEgde(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    QPointF getRelCenterPosition();
    void updatePath();
    MovablePoint *qra_getPointAt(const QPointF &absPtPos,
                                 const CanvasMode &currentCanvasMode,
                                 const qreal &canvasScaleInv);
    QPainterPath getCurrentPath() {
        return mPath;
    }

    void prp_loadFromSql(const int &boundingBoxId);
    int prp_saveToSql(QSqlQuery *query,
                      const int &boundingBoxId);
    void makeDuplicate(Property *property) {
        duplicatePathsTo((PathAnimator*)property);
    }

    Property *makeDuplicate() {
        PathAnimator *newAnimator = new PathAnimator();
        makeDuplicate(newAnimator);
        return newAnimator;
    }


    PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
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

    bool SWT_isPathAnimator() { return true; }
private:
    BoundingBox *mParentBox = NULL;
    QPainterPath mPath;
    QList<SinglePathAnimator*> mSinglePaths;
signals:
    void lastSinglePathRemoved();
};

#endif // PATHANIMATOR_H
