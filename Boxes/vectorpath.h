#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include <QLinearGradient>
#include "pathbox.h"

class PathPoint;
class BoxesGroup;
class PathAnimator;
typedef QSharedPointer<PathAnimator> PathAnimatorQSPtr;

enum CanvasMode : short;

class VectorPathEdge;

class VectorPath : public PathBox {
public:
    VectorPath();
    static VectorPath *createPathFromSql(int boundingBoxId);

    ~VectorPath();

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    int saveToSql(QSqlQuery *query, const int &parentId);

    PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                        const bool &adjust,
                                        const qreal &canvasScaleInv);

    virtual void prp_loadFromSql(const int &boundingBoxId);

    void applyCurrentTransformation();
    VectorPathEdge *getEgde(const QPointF &absPos,
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
protected:
    SkPath getPathAtRelFrame(const int &relFrame);
    PathAnimatorQSPtr mPathAnimator;

    void loadPointsFromSql(int boundingBoxId);
};

#endif // VECTORPATH_H
