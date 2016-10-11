#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "boundingbox.h"
#include "pathpoint.h"
#include <QLinearGradient>
#include "gradientpoint.h"
#include "pathanimator.h"

class BoxesGroup;

class MainWindow;

class PathPivot;

enum CanvasMode : short;

class VectorPath;

class Edge;

class GradientPoints : public ComplexAnimator {
public:
    GradientPoints();
    ~GradientPoints();

    void initialize(VectorPath *parentT, QPointF startPt = QPointF(0., 0.),
                    QPointF endPt = QPointF(100., 100.));

    void initialize(VectorPath *parentT,
                    int fillGradientStartId, int fillGradientEndId);

    void clearAll();

    void enable();

    void disable();

    void drawGradientPoints(QPainter *p);

    MovablePoint *getPointAt(QPointF absPos);

    QPointF getStartPoint();

    QPointF getEndPoint();

    void setColors(QColor startColor, QColor endColor);

    bool enabled;
    GradientPoint *startPoint;
    GradientPoint *endPoint;
    VectorPath *parent;

    void attachToBoneFromSqlZId();

    void setPositions(QPointF startPos, QPointF endPos, bool saveUndoRedo = true);
};

class VectorPath : public BoundingBox
{
public:
    VectorPath(BoxesGroup *group);
    VectorPath(int boundingBoxId, BoxesGroup *parent);

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
    void centerPivotPosition();

    void resetStrokeGradientPointsPos(bool finish) {
        mStrokeGradientPoints.setRecording(false);
        mStrokeGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), finish);
    }

    void resetFillGradientPointsPos(bool finish) {
        mFillGradientPoints.setRecording(false);
        mFillGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), finish);
    }

    void changeFillGradient(bool wasGradient, bool isGradient, bool finish) {
        if(wasGradient) {
            mFillPaintSettings.getGradient()->removePath(this);
        } else if(isGradient) {
            resetFillGradientPointsPos(finish);
        }
    }

    void changeStrokeGradient(bool wasGradient, bool isGradient, bool finish) {
        if(wasGradient) {
            mStrokeSettings.getGradient()->removePath(this);
        } else if(isGradient) {
            resetStrokeGradientPointsPos(finish);
        }
    }

    virtual void setFillGradient(Gradient* gradient, bool finish) {
        if(mFillPaintSettings.getPaintType() != GRADIENTPAINT) {
            setFillPaintType(GRADIENTPAINT,
                              mFillPaintSettings.getCurrentColor(),
                              gradient);
        } else {
            changeFillGradient(mFillPaintSettings.getPaintType() == GRADIENTPAINT,
                                 true, finish);

            mFillPaintSettings.setGradient(gradient);
            updateDrawGradients();
        }
    }

    virtual void setStrokeGradient(Gradient* gradient, bool finish) {
        if(mStrokeSettings.getPaintType() != GRADIENTPAINT) {
            setStrokePaintType(GRADIENTPAINT,
                               mStrokeSettings.getCurrentColor(),
                               gradient);
        } else {
            changeStrokeGradient(mStrokeSettings.getPaintType() == GRADIENTPAINT,
                                 true, finish);

            mStrokeSettings.setGradient(gradient);
            updateDrawGradients();
        }
    }

    virtual void setFillFlatColor(Color color, bool finish) {
        if(mFillPaintSettings.getPaintType() != FLATPAINT) {
            setFillPaintType(FLATPAINT, color, NULL);
        } else {
            mFillPaintSettings.setCurrentColor(color);
            if(finish) {
                mFillPaintSettings.getColorAnimator()->finishTransform();
            }


        }
    }

    virtual void setStrokeFlatColor(Color color, bool finish) {
        if(mStrokeSettings.getPaintType() != FLATPAINT) {
            setStrokePaintType(FLATPAINT, color, NULL);
        } else {
            mStrokeSettings.setCurrentColor(color);
            if(finish) {
                mStrokeSettings.getColorAnimator()->finishTransform();
            }


        }
    }

    virtual void setFillPaintType(PaintType paintType, Color color,
                                  Gradient* gradient) {
        changeFillGradient(mFillPaintSettings.getPaintType() == GRADIENTPAINT,
                           paintType == GRADIENTPAINT, true);
        if(paintType == GRADIENTPAINT) {
            mFillPaintSettings.setGradient(gradient);
        } else if(paintType == FLATPAINT) {
            mFillPaintSettings.setCurrentColor(color);
        }
        mFillPaintSettings.setPaintType(paintType);

        updateDrawGradients();
    }

    virtual void setStrokePaintType(PaintType paintType, Color color,
                                    Gradient* gradient) {
        changeStrokeGradient(mStrokeSettings.getPaintType() == GRADIENTPAINT,
                           paintType == GRADIENTPAINT, true);
        if(paintType == GRADIENTPAINT) {
            mStrokeSettings.setGradient(gradient);
            updateDrawGradients();
        } else if(paintType == FLATPAINT) {
            mStrokeSettings.setCurrentColor(color);
        }
        mStrokeSettings.setPaintType(paintType);


    }

    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle) {
        mStrokeSettings.setCapStyle(capStyle);
        updateOutlinePath();


    }

    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) {
        mStrokeSettings.setJoinStyle(joinStyle);
        updateOutlinePath();

    }

    virtual void setStrokeWidth(qreal strokeWidth, bool finish) {
        mStrokeSettings.setCurrentStrokeWidth(strokeWidth);
        if(finish) {
            mStrokeSettings.getStrokeWidthAnimator()->finishTransform();
        }
    }

    void startStrokeWidthTransform() {
        mStrokeSettings.getStrokeWidthAnimator()->startTransform();
    }

    void startStrokeColorTransform() {
        mStrokeSettings.getColorAnimator()->startTransform();
    }

    void startFillColorTransform() {
        mFillPaintSettings.getColorAnimator()->startTransform();
    }

    const StrokeSettings *getStrokeSettings();
    const PaintSettings *getFillSettings();
    void updateDrawGradients();

    PathPoint *addPointRelPos(QPointF relPos,
                              QPointF startRelPos, QPointF endRelPos,
                              PathPoint *toPoint = NULL);
    int saveToSql(int parentId);

    void clearAll();

    PathPoint *createNewPointOnLineNear(QPointF absPos, bool adjust);
    qreal percentAtPoint(QPointF absPos, qreal distTolerance,
                         qreal maxPercent, qreal minPercent,
                         bool *found = NULL, QPointF *posInPath = NULL);
    PathPoint *findPointNearestToPercent(qreal percent, qreal *foundAtPercent);

    void attachToBoneFromSqlZId();

    void updateAfterFrameChanged(int currentFrame);

    void startAllPointsTransform();
    void finishAllPointsTransform();

    Edge *getEgde(QPointF absPos);

    void updateOutlinePath();
    void scheduleOutlinePathUpdate();
    void updateOutlinePathIfNeeded();
    void setRenderCombinedTransform();

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
    Edge *getEdgeFromMappedEditPath(QPointF absPos);
    Edge *getEdgeFromMappedPath(QPointF absPos);
    void deletePointAndApproximate(PathPoint *pointToRemove);
protected:
    bool mInfluenceEnabled = false;

    void updatePathPointIds();
    PathAnimator mPathAnimator;

    void loadPointsFromSql(int vectorPathId);

    GradientPoints mFillGradientPoints;
    GradientPoints mStrokeGradientPoints;

    QLinearGradient mDrawFillGradient;
    QLinearGradient mDrawStrokeGradient;
    QPen mDrawPen;

    PaintSettings mFillPaintSettings;
    StrokeSettings mStrokeSettings;

    void updatePath();
    void updateMappedPath();

    bool mPathUpdateNeeded = false;
    bool mMappedPathUpdateNeeded = false;
    bool mOutlinePathUpdateNeeded = false;

    bool mClosedPath = false;
    QList<PathPoint*> mSeparatePaths;
    QList<PathPoint*> mPoints;
    QPainterPath mEditPath;
    QPainterPath mPath;
    QPainterPath mMappedEditPath;
    QPainterPath mMappedPath;
    QPainterPath mMappedOutlinePath;
    QPainterPathStroker mPathStroker;
    QPainterPath mMappedWhole;
    void updateWholePath();
    qreal findPercentForPoint(QPointF point, qreal minPercent = 0.,
                              qreal maxPercent = 1.);
protected:
    void updateAfterTransformationChanged();
    void updateAfterCombinedTransformationChanged();
};

#endif // VECTORPATH_H
