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

class VectorPath : public PathBox
{
public:
    VectorPath(BoxesGroup *group);
    static VectorPath *createPathFromSql(int boundingBoxId,
                                         BoxesGroup *parent);

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

//    void disconnectPoints(PathPoint *point1, PathPoint *point2);
//    void connectPoints(PathPoint *point1, PathPoint *point2);
    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    PathAnimator *getPathAnimator();
    void duplicatePathAnimatorFrom(PathAnimator *source);
    void updatePath();

    bool SWT_isVectorPath() { return true; }
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
protected:
    PathAnimatorQSPtr mPathAnimator;

    void loadPointsFromSql(int boundingBoxId);
};

#endif // VECTORPATH_H
