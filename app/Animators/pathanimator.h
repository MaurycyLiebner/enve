#ifndef PATHANIMATOR_H
#define PATHANIMATOR_H
#include <QPainterPath>
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"

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

    void readProperty(QIODevice *target);
    void writeProperty(QIODevice * const target) const;
    bool SWT_isPathAnimator() const;

    VectorPathEdge *getEdge(const QPointF &absPos,
                            const qreal &canvasScaleInv);
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                 const CanvasMode &currentCanvasMode,
                                 const qreal &canvasScaleInv);

    NodePoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);
    void applyTransformToPoints(const QMatrix &transform);

    void drawSelected(SkCanvas *canvas,
                      const CanvasMode &currentCanvasMode,
                      const SkScalar &invScale,
                      const SkMatrix &combinedTransform);

    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>> &list);

    BoundingBox *getParentBox();

    void loadPathFromSkPath(const SkPath &path);

    void setParentBox(BoundingBox *parent);
    void addSinglePathAnimator(const qsptr<VectorPathAnimator> &path);
    void removeSinglePathAnimator_k(const qsptr<VectorPathAnimator> &path);
    void selectAllPoints(Canvas * const canvas);
    SkPath getPathAtRelFrame(const int &relFrame);
    SkPath getPathAtRelFrameF(const qreal &relFrame);
    void shiftAllPoints(const int &by);
    void revertAllPoints();
    void shiftAllPointsForAllKeys(const int &by);
    void revertAllPointsForAllKeys();
    void readVectorPathAnimator(QIODevice *target);
    void moveAllSinglePathsToAnimator_k(PathAnimator * const target);
    const QList<qsptr<VectorPathAnimator>> &getSinglePathsList() {
        return mSinglePaths;
    }

//    bool prp_differencesBetweenRelFrames(const int &relFrame1,
//                                         const int &relFrame2) {
//        bool interpolate = mSmoothTransformation->g;
//    }
    QMatrix getCombinedTransform();
private:
    BoundingBox *mParentBox = nullptr;
    QList<qsptr<VectorPathAnimator>> mSinglePaths;
signals:
    void lastSinglePathRemoved();
};

#endif // PATHANIMATOR_H
