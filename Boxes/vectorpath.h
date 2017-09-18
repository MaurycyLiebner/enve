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

    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate();
    PathAnimator *getPathAnimator();
    void duplicatePathAnimatorFrom(PathAnimator *source);

    bool SWT_isVectorPath() { return true; }
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
    SkPath getPathAtRelFrame(const int &relFrame);

    void selectAllPoints(Canvas *canvas);
    void writeBoundingBox(QFile *file);
    void readBoundingBox(QFile *file);
protected:
    PathAnimatorQSPtr mPathAnimator;
};

#endif // VECTORPATH_H
