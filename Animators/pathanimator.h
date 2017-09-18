#ifndef PATHANIMATOR_H
#define PATHANIMATOR_H
#include <QPainterPath>
#include "complexanimator.h"
#include "skiaincludes.h"

enum CanvasMode : short;

class VectorPathEdge;
class MovablePoint;
class NodePoint;
class VectorPathAnimator;
class BoundingBox;
class Canvas;

class PathAnimator : public ComplexAnimator {
    Q_OBJECT
public:
    PathAnimator(BoundingBox *parentBox);
    PathAnimator();

    ~PathAnimator();

    VectorPathEdge *getEdge(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                 const CanvasMode &currentCanvasMode,
                                 const qreal &canvasScaleInv);

    void makeDuplicate(Property *property);

    Property *makeDuplicate();


    NodePoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
    void applyTransformToPoints(const QMatrix &transform);

    void duplicatePathsTo(PathAnimator *target);

    void drawSelected(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const SkScalar &invScale,
                      const SkMatrix &combinedTransform);

    void selectAndAddContainedPointsToList(
            const QRectF &absRect, QList<MovablePoint *> *list);

    BoundingBox *getParentBox();

    void loadPathFromSkPath(const SkPath &path);

    void setParentBox(BoundingBox *parent);
    void addSinglePathAnimator(VectorPathAnimator *path,
                               const bool &saveUndoRedo = true);
    void removeSinglePathAnimator(VectorPathAnimator *path,
                                  const bool &saveUndoRedo = true);
    void selectAllPoints(Canvas *canvas);
    bool SWT_isPathAnimator();
    SkPath getPathAtRelFrame(const int &relFrame);
    void readPathAnimator(QFile *file);
    void writePathAnimator(QFile *file);
private:
    BoundingBox *mParentBox = NULL;
    QList<VectorPathAnimator*> mSinglePaths;
signals:
    void lastSinglePathRemoved();
};

#endif // PATHANIMATOR_H
