#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "pathpoint.h"
#include <QLinearGradient>
#include "Animators/pathanimator.h"
#include "Boxes/pathbox.h"

class BoxesGroup;

class MainWindow;

class PathPivot;

enum CanvasMode : short;

class Edge;

class VectorShapesMenu;

class VectorPath : public PathBox
{
public:
    VectorPath(BoxesGroup *group);
    static VectorPath *createPathFromSql(int boundingBoxId, BoxesGroup *parent);

    ~VectorPath();

    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);

    PathPoint *addPointAbsPos(QPointF absPtPos, PathPoint *toPoint = NULL);
    PathPoint *addPointRelPos(QPointF relPtPos, PathPoint *toPoint = NULL);

    MovablePoint *getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode);

    void selectAndAddContainedPointsToList(QRectF absRect, QList<MovablePoint*> *list);

    PathPoint *addPoint(PathPoint *pointToAdd, PathPoint *toPoint);

    void removePoint(PathPoint *point);
    void replaceSeparatePathPoint(PathPoint *pointBeingReplaced, PathPoint *newPoint);
    void addPointToSeparatePaths(PathPoint *pointToAdd, bool saveUndoRedo = true);
    void removePointFromSeparatePaths(PathPoint *pointToRemove, bool saveUndoRedo = true);
    void appendToPointsList(PathPoint *point, bool saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point, bool saveUndoRedo = true);

    PathPoint *addPointRelPos(QPointF relPos,
                              QPointF startRelPos, QPointF endRelPos,
                              PathPoint *toPoint = NULL);
    int saveToSql(QSqlQuery *query, int parentId);

    PathPoint *createNewPointOnLineNear(QPointF absPos, bool adjust);
    qreal percentAtPoint(QPointF absPos, qreal distTolerance,
                         qreal maxPercent, qreal minPercent,
                         bool *found = NULL, QPointF *posInPath = NULL);

    void updateAfterFrameChanged(int currentFrame);

    void startAllPointsTransform();
    void finishAllPointsTransform();

    void showContextMenu(QPoint globalPos);

    void deletePointAndApproximate(PathPoint *pointToRemove);
    virtual void loadFromSql(int boundingBoxId);
    void saveCurrentPathToShape(VectorPathShape *shape);
    void addShapesToShapesMenu(VectorShapesMenu *menu);
    VectorPathShape *createNewShape(bool relative);
    void removeShape(VectorPathShape *shape, bool saveUndoRedo = true);
    void editShape(VectorPathShape *shape);
    void finishEditingShape(VectorPathShape *shape);
    void cancelEditingShape();
    void addShape(VectorPathShape *shape, bool saveUndoRedo = true);
    void applyTransformToPoints(QMatrix transform);
    void applyCurrentTransformation();
    Edge *getEgde(QPointF absPos);

    void loadPathFromQPainterPath(const QPainterPath &path);
protected:
    ComplexAnimator *mShapesAnimator = NULL;

    QList<VectorPathShape*> mShapes;

    void updatePath();
    void updatePathPointIds();
    PathAnimator mPathAnimator;

    void loadPointsFromSql(int boundingBoxId);

    qreal findPercentForPoint(QPointF point,
                              PathPoint **prevPoint,
                              qreal *error);

    bool mClosedPath = false;

    QList<PathPoint*> mSeparatePaths;
    QList<PathPoint*> mPoints;
    QPainterPath mEditPath;
    QPointF getRelCenterPosition();

    bool mShapesEnabled = false;
    VectorPathShape *mCurrentEditedShape = NULL;
    bool getTAndPointsForMouseEdgeInteraction(const QPointF &absPos,
                                              qreal *pressedT,
                                              PathPoint **prevPoint,
                                              PathPoint **nextPoint);
};

#endif // VECTORPATH_H
