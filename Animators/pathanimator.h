#ifndef PATHANIMATOR_H
#define PATHANIMATOR_H
#include <QPainterPath>
#include "complexanimator.h"
#include "skiaincludes.h"

enum CanvasMode : short;

class VectorPathEdge;
class MovablePoint;
class PathPoint;
class SinglePathAnimator;
class BoundingBox;

class PathAnimator : public ComplexAnimator {
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
    const QPainterPath &getCurrentPath();
    const SkPath &getCurrentSkPath();

    void prp_loadFromSql(const int &boundingBoxId);
    int prp_saveToSql(QSqlQuery *query,
                      const int &boundingBoxId);
    void makeDuplicate(Property *property);

    Property *makeDuplicate();


    PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
    void updateAfterFrameChanged(const int &currentFrame);
    qreal findPercentForPoint(const QPointF &point,
                              PathPoint **prevPoint,
                              qreal *error);
    void applyTransformToPoints(const QMatrix &transform);

    void duplicatePathsTo(PathAnimator *target);

    void drawSelected(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const SkScalar &invScale,
                      const SkMatrix &combinedTransform);

    void selectAndAddContainedPointsToList(
            const QRectF &absRect, QList<MovablePoint *> *list);

    BoundingBox *getParentBox();

    void loadPathFromQPainterPath(const QPainterPath &path);
    void loadPathFromSkPath(const SkPath &path);

    void setParentBox(BoundingBox *parent);
    void addSinglePathAnimator(SinglePathAnimator *path,
                               const bool &saveUndoRedo = true);
    void removeSinglePathAnimator(SinglePathAnimator *path,
                                  const bool &saveUndoRedo = true);

    bool SWT_isPathAnimator();
private:
    BoundingBox *mParentBox = NULL;
    QPainterPath mPath;
    SkPath mSkPath;
    QList<SinglePathAnimator*> mSinglePaths;
signals:
    void lastSinglePathRemoved();
};

#endif // PATHANIMATOR_H
