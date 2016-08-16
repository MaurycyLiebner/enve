#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "boundingbox.h"
#include "pathpoint.h"
#include <QLinearGradient>
#include "gradientpoint.h"

class BoxesGroup;

class MainWindow;

class PathPivot;

enum CanvasMode : short;

class VectorPath;

class GradientPoints {
public:
    GradientPoints();

    void initialize(VectorPath *parentT);

    void enable();

    void disable();

    void draw(QPainter *p);

    MovablePoint *getPointAt(QPointF absPos);

    QPointF getStartPoint();

    QPointF getEndPoint();

    void setColors(QColor startColor, QColor endColor);

    bool enabled;
    GradientPoint *startPoint;
    GradientPoint *endPoint;
    VectorPath *parent;

    void setPositions(QPointF startPos, QPointF endPos, bool saveUndoRedo = true);
};

class VectorPath : public BoundingBox
{
public:
    VectorPath(BoxesGroup *group);
    ~VectorPath();

    virtual QRectF getBoundingRect();
    void draw(QPainter *p);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);

    PathPoint *addPointAbsPos(QPointF absPtPos, PathPoint *toPoint = NULL);
    PathPoint *addPointRelPos(QPointF relPtPos, PathPoint *toPoint = NULL);

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

    void setStrokeSettings(StrokeSettings strokeSettings, bool saveUndoRedo = true);
    void setFillSettings(PaintSettings fillSettings, bool saveUndoRedo = true);

    StrokeSettings getStrokeSettings();
    PaintSettings getFillSettings();
    void updateDrawGradients();

    void startStrokeTransform();
    void startFillTransform();
    void finishStrokeTransform();
    void finishFillTransform();
    PathPoint *addPointRelPos(QPointF relPos,
                              QPointF startRelPos, QPointF endRelPos,
                              PathPoint *toPoint = NULL);
    void saveToQuery(QSqlQuery *query, qint32 parentId);
private:
    GradientPoints mFillGradientPoints;
    GradientPoints mStrokeGradientPoints;

    QLinearGradient mDrawFillGradient;
    QLinearGradient mDrawStrokeGradient;
    QPen mDrawPen;

    PaintSettings mFillPaintSettings;
    StrokeSettings mStrokeSettings;

    PaintSettings mSavedFillPaintSettings;
    StrokeSettings mSavedStrokeSettings;

    void updatePath();
    void updateMappedPath();

    bool mPathUpdateNeeded = false;
    bool mMappedPathUpdateNeeded = false;

    bool mClosedPath = false;
    QList<PathPoint*> mSeparatePaths;
    QList<PathPoint*> mPoints;
    QPainterPath mPath;
    QPainterPath mMappedPath;
    QPainterPath mOutlinePath;
    QPainterPathStroker mPathStroker;
    QPainterPath mMappedWhole;
    void updateOutlinePath();
    void updateWholePath();
protected:
    void updateAfterTransformationChanged();
    void updateAfterCombinedTransformationChanged();
};

#endif // VECTORPATH_H
