#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "pathpoint.h"
#include <QLinearGradient>
#include "pathanimator.h"
#include "pathbox.h"

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
    int saveToSql(int parentId);

    PathPoint *createNewPointOnLineNear(QPointF absPos, bool adjust);
    qreal percentAtPoint(QPointF absPos, qreal distTolerance,
                         qreal maxPercent, qreal minPercent,
                         bool *found = NULL, QPointF *posInPath = NULL);
    PathPoint *findPointNearestToPercent(qreal percent, qreal *foundAtPercent);

    void updateAfterFrameChanged(int currentFrame);

    void startAllPointsTransform();
    void finishAllPointsTransform();

    Edge *getEgde(QPointF absPos);

    void setInfluenceEnabled(bool bT) {
        if(bT) {
            enableInfluence();
        } else {
            disableInfluence();
        }
    }

    void disableInfluence() {
        mInfluenceEnabled = false;

        foreach(PathPoint *point, mPoints) {
            point->disableInfluenceAnimators();
        }
    }

    void enableInfluence() {
        mInfluenceEnabled = true;
        schedulePathUpdate();

        foreach(PathPoint *point, mPoints) {
            point->enableInfluenceAnimators();
        }
    }

    void showContextMenu(QPoint globalPos);

    PathPoint *findPointNearestToPercentEditPath(qreal percent, qreal *foundAtPercent);
    qreal findPercentForPointEditPath(QPointF point, qreal minPercent = 0., qreal maxPercent = 1.);
    Edge *getEdgeFromEditPath(QPointF absPos);
    Edge *getEdgeFromPath(QPointF absPos);
    void deletePointAndApproximate(PathPoint *pointToRemove);
    virtual void loadFromSql(int boundingBoxId);
    void saveCurrentPathToShape(VectorPathShape *shape);
    void addShapesToShapesMenu(VectorShapesMenu *menu);
    VectorPathShape *createNewShape();
    void removeShape(VectorPathShape *shape);
    void editShape(VectorPathShape *shape);
    void finishEditingShape(VectorPathShape *shape);
    void cancelEditingShape();
protected:
    ComplexAnimator *mShapesAnimator = NULL;

    QList<VectorPathShape*> mShapes;

    bool mInfluenceEnabled = false;
    void updatePath();
    void updatePathPointIds();
    PathAnimator mPathAnimator;

    void loadPointsFromSql(int boundingBoxId);

    qreal findPercentForPoint(QPointF point, qreal minPercent = 0.,
                              qreal maxPercent = 1.);

    bool mClosedPath = false;

    QList<PathPoint*> mSeparatePaths;
    QList<PathPoint*> mPoints;
    QPainterPath mEditPath;
    void centerPivotPosition();

    bool mShapesEnabled = false;
    VectorPathShape *mCurrentEditedShape = NULL;
};

#endif // VECTORPATH_H
