#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "childparent.h"
#include "pathpoint.h"

class Canvas;

enum CanvasMode : short;

class VectorPath : public ChildParent
{
public:
    VectorPath(Canvas *canvasT);
    ~VectorPath();

    void updatePath();
    void updateAfterTransformationChanged();
    void updateAfterCombinedTransformationChanged();
    void updateMappedPath();

    QRectF getBoundingRect();
    void draw(QPainter *p);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);

    PathPoint *addPoint(QPointF absPtPos, PathPoint *toPoint = NULL);

    bool pointInsidePath(QPointF point);
    PathPoint *getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode);

    void addContainedPointsToList(QRectF absRect, QList<PathPoint*> *list);
    bool isContainedIn(QRectF absRect);

    bool isSelected();
    void select();
    void deselect();
    PathPoint *addPoint(PathPoint *pointToAdd, PathPoint *toPoint);

    Canvas *getCanvas();

private:
    bool mSelected = false;
    bool mClosedPath = false;
    QList<PathPoint*> mSeparatePaths;
    QList<PathPoint*> mPoints;
    QPainterPath mPath;
    QPainterPath mMappedPath;
};

#endif // VECTORPATH_H
