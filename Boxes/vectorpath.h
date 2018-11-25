#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include <QLinearGradient>
#include "pathbox.h"

class NodePoint;
class BoxesGroup;
class PathAnimator;
typedef QSharedPointer<PathAnimator> PathAnimatorQSPtr;

enum CanvasMode : short;

class VectorPathEdge;

class VectorPath : public PathBox {
public:
    VectorPath();

    ~VectorPath();

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);


    NodePoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);

    void applyCurrentTransformation();
    VectorPathEdge *getEdge(const QPointF &absPos,
                            const qreal &canvasScaleInv);

    void loadPathFromSkPath(const SkPath &path);

    PathAnimator *getPathAnimator();

    bool SWT_isVectorPath() { return true; }
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePointSPtr> &list);
    SkPath getPathAtRelFrame(const int &relFrame);
    SkPath getPathAtRelFrameF(const qreal &relFrame);


    void selectAllPoints(Canvas *canvas);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void shiftAllPointsForAllKeys(const int &by);
    void revertAllPointsForAllKeys();
    void shiftAllPoints(const int &by);
    void revertAllPoints();
    void breakPathsApart();

    bool differenceInEditPathBetweenFrames(
                const int& frame1, const int& frame2) const;
protected:
    void getMotionBlurProperties(QList<PropertyQSPtr>& list);
    PathAnimatorQSPtr mPathAnimator;
};

#endif // VECTORPATH_H
