#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "childparent.h"
#include "pathpoint.h"

class Canvas;

class MainWindow;

enum CanvasMode : short;

class VectorPath : public ChildParent
{
public:
    VectorPath(Canvas *canvasT);
    ~VectorPath();

    QRectF getBoundingRect();
    void draw(QPainter *p);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);

    PathPoint *addPoint(QPointF absPtPos, PathPoint *toPoint = NULL);

    bool pointInsidePath(QPointF point);
    MovablePoint *getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode);

    void selectAndAddContainedPointsToList(QRectF absRect, QList<MovablePoint*> *list);
    bool isContainedIn(QRectF absRect);

    bool isSelected();
    void select();
    void deselect();
    PathPoint *addPoint(PathPoint *pointToAdd, PathPoint *toPoint);

    Canvas *getCanvas();

    void removePoint(PathPoint *point);
    void scheduleRepaint();
    void replaceSeparatePathPoint(PathPoint *pointBeingReplaced, PathPoint *newPoint);
    void addPointToSeparatePaths(PathPoint *pointToAdd, bool saveUndoRedo = true);
    void removePointFromSeparatePaths(PathPoint *pointToRemove, bool saveUndoRedo = true);
    void appendToPointsList(PathPoint *point, bool saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point, bool saveUndoRedo = true);
    void remove();

    void schedulePathUpdate();
    void scheduleMappedPathUpdate();

    void updatePathIfNeeded();
    void updateMappedPathIfNeeded();
private:
    void updatePath();
    void updateMappedPath();

    bool mPathUpdateNeeded = false;
    bool mMappedPathUpdateNeeded = false;

    bool mSelected = false;
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
