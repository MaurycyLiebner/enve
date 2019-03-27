#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include <QLinearGradient>
#include "pathbox.h"

class NodePoint;
class BoxesGroup;
class PathAnimator;

enum CanvasMode : short;

class VectorPathEdge;

class VectorPath : public PathBox {
public:
    VectorPath();

    ~VectorPath();

    void drawCanvasControls(SkCanvas * const canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);


    MovablePoint *createNewPointOnLineNear(const QPointF &absPos,
                                           const bool &adjust,
                                           const qreal &canvasScaleInv);

    void applyCurrentTransformation();
    VectorPathEdge *getEdge(const QPointF &absPos,
                            const qreal &canvasScaleInv);

    void loadPathFromSkPath(const SkPath &path);

    PathAnimator *getPathAnimator();

    bool SWT_isVectorPath() const { return true; }
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>> &list);
    SkPath getPathAtRelFrameF(const qreal &relFrame);


    void selectAllPoints(Canvas * const canvas);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void shiftAllPointsForAllKeys(const int &by);
    void revertAllPointsForAllKeys();
    void shiftAllPoints(const int &by);
    void revertAllPoints();
    void breakPathsApart_k();

    bool differenceInEditPathBetweenFrames(
                const int& frame1, const int& frame2) const;
protected:
    void getMotionBlurProperties(QList<Property*> &list) const;
    qsptr<PathAnimator> mPathAnimator;
};

#endif // VECTORPATH_H
