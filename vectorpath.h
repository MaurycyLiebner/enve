#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "boundingbox.h"
#include "pathpoint.h"

class BoxesGroup;

class MainWindow;

class PathPivot;

enum CanvasMode : short;

class VectorPath : public BoundingBox
{
public:
    VectorPath(BoxesGroup *group);
    ~VectorPath();

    virtual QRectF getBoundingRect();
    void draw(QPainter *p);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);

    PathPoint *addPoint(QPointF absPtPos, PathPoint *toPoint = NULL);

    bool pointInsidePath(QPointF point);
    MovablePoint *getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode);

    void selectAndAddContainedPointsToList(QRectF absRect, QList<MovablePoint*> *list);

    PathPoint *addPoint(PathPoint *pointToAdd, PathPoint *toPoint);

    void removePoint(PathPoint *point);
    void replaceSeparatePathPoint(PathPoint *pointBeingReplaced, PathPoint *newPoint);
    void addPointToSeparatePaths(PathPoint *pointToAdd, bool saveUndoRedo = true);
    void removePointFromSeparatePaths(PathPoint *pointToRemove, bool saveUndoRedo = true);
    void appendToPointsList(PathPoint *point, bool saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point, bool saveUndoRedo = true);

    void schedulePathUpdate();
    void scheduleMappedPathUpdate();

    void updatePathIfNeeded();
    void updateMappedPathIfNeeded();
    void updatePivotPosition();
private:
    void updatePath();
    void updateMappedPath();

    bool mPathUpdateNeeded = false;
    bool mMappedPathUpdateNeeded = false;

    bool mClosedPath = false;
    QList<PathPoint*> mSeparatePaths;
    QList<PathPoint*> mPoints;
    QPainterPath mPath;
    QPainterPath mMappedPath;
protected:
    void updateAfterTransformationChanged();
    void updateAfterCombinedTransformationChanged();
};

#endif // VECTORPATH_H
