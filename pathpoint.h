#ifndef PATHPOINT_H
#define PATHPOINT_H
#include "movablepoint.h"
#include <QSqlQuery>
#include "pointhelpers.h"

class UndoRedoStack;

class VectorPath;

class CtrlPoint;

enum CanvasMode : short;

class PathPoint;

struct PathPointValues {
    PathPointValues(QPointF startPosT,
                    QPointF pointPosT,
                    QPointF endPosT) {
        startRelPos = startPosT;
        pointRelPos = pointPosT;
        endRelPos = endPosT;
    }

    PathPointValues() {}


    QPointF startRelPos;
    QPointF pointRelPos;
    QPointF endRelPos;

    PathPointValues &operator/=(const qreal &val)
    {
        startRelPos /= val;
        pointRelPos /= val;
        endRelPos /= val;
        return *this;
    }
    PathPointValues &operator*=(const qreal &val)
    {
        startRelPos *= val;
        pointRelPos *= val;
        endRelPos *= val;
        return *this;
    }
    PathPointValues &operator+=(const PathPointValues &ppv)
    {
        startRelPos += ppv.startRelPos;
        pointRelPos += ppv.pointRelPos;
        endRelPos += ppv.endRelPos;
        return *this;
    }
    PathPointValues &operator-=(const PathPointValues &ppv)
    {
        startRelPos -= ppv.startRelPos;
        pointRelPos -= ppv.pointRelPos;
        endRelPos -= ppv.endRelPos;
        return *this;
    }
};

PathPointValues operator+(const PathPointValues &ppv1, const PathPointValues &ppv2);
PathPointValues operator-(const PathPointValues &ppv1, const PathPointValues &ppv2);
PathPointValues operator/(const PathPointValues &ppv, const qreal &val);
PathPointValues operator*(const PathPointValues &ppv, const qreal &val);
PathPointValues operator*(const qreal &val, const PathPointValues &ppv);

class VectorPathShape : public SmartPointerTarget {
public:
    VectorPathShape() : SmartPointerTarget() {
        mInfluence.setValueRange(0., 1.);
        mInfluence.blockPointer();
        mInfluence.setName("influence");
    }

    QString getName() {
        return mName;
    }

    void setName(QString name) {
        mName = name;
        mInfluence.setName(name);
    }

    bool isRelative() {
        return mRelative;
    }

    void setRelative(bool bT) {
        mRelative = bT;
    }

    qreal getCurrentInfluence() {
        return mInfluence.getCurrentValue();
    }

    void setCurrentInfluence(qreal value, bool finish = false) {
        mInfluence.setCurrentValue(value, finish);
    }

    QrealAnimator *getInfluenceAnimator() {
        return &mInfluence;
    }

    void setNumberPoints(int n) {
        for(int i = 0; i < n; i++) {
            mPointsValues << new PathPointValues();
        }
    }

    PathPointValues *getPathPointValuesForPoint(int pointId) {
        return mPointsValues.at(pointId);
    }

private:
    QList<PathPointValues*> mPointsValues;
    bool mRelative = false;
    QString mName;
    QrealAnimator mInfluence;
};

class PointShapeValues {
public:
    PointShapeValues();
    PointShapeValues(VectorPathShape *shape, int pointId) {
        mShape = shape;
        mValues = shape->getPathPointValuesForPoint(pointId);
    }

    const PathPointValues &getValues() const {
        return *mValues;
    }

    VectorPathShape *getParentShape() const {
        return mShape;
    }

    void setPointValues(const PathPointValues &values) {
        *mValues = values;
    }
private:
    PathPointValues *mValues = NULL;
    VectorPathShape *mShape = NULL;
};

class PathPointAnimators : public ComplexAnimator {
public:
    PathPointAnimators() : ComplexAnimator() {
        setName("point");
    }

    void setAllVars(PathPoint *parentPathPointT,
                    QPointFAnimator *endPosAnimatorT,
                    QPointFAnimator *startPosAnimatorT,
                    QPointFAnimator *pathPointPosAnimatorT) {
        parentPathPoint = parentPathPointT;
        endPosAnimator = endPosAnimatorT;
        endPosAnimator->setName("ctrl pt 1 pos");
        startPosAnimator = startPosAnimatorT;
        startPosAnimator->setName("ctrl pt 2 pos");
        pathPointPosAnimator = pathPointPosAnimatorT;
        pathPointPosAnimator->setName("point pos");

        addChildAnimator(pathPointPosAnimator);
        addChildAnimator(endPosAnimator);
        addChildAnimator(startPosAnimator);
    }

    bool isOfPathPoint(PathPoint *checkPoint) {
        return parentPathPoint == checkPoint;
    }
private:
    PathPoint *parentPathPoint;
    QPointFAnimator *endPosAnimator;
    QPointFAnimator *startPosAnimator;
    QPointFAnimator *pathPointPosAnimator;
}; 

class PathPoint : public MovablePoint
{
public:
    PathPoint(VectorPath *vectorPath);

    ~PathPoint();

    void applyTransform(QMatrix transform);

    void startTransform();
    void finishTransform();

    void moveByRel(QPointF relTranslation);

    QPointF getStartCtrlPtAbsPos() const;
    QPointF getStartCtrlPtValue() const;
    CtrlPoint *getStartCtrlPt();

    QPointF getEndCtrlPtAbsPos();
    QPointF getEndCtrlPtValue() const;
    CtrlPoint *getEndCtrlPt();

    void draw(QPainter *p, const CanvasMode &mode);

    PathPoint *getNextPoint();
    PathPoint *getPreviousPoint();

    bool isEndPoint();

    void setPointAsPrevious(PathPoint *pointToSet, bool saveUndoRedo = true);
    void setPointAsNext(PathPoint *pointToSet, bool saveUndoRedo = true);
    void setNextPoint(PathPoint *mNextPoint, bool saveUndoRedo = true);
    void setPreviousPoint(PathPoint *mPreviousPoint, bool saveUndoRedo = true);

    bool hasNextPoint();
    bool hasPreviousPoint();

    PathPoint *addPointAbsPos(QPointF absPos);
    PathPoint *addPoint(PathPoint *pointToAdd);

    void connectToPoint(PathPoint *point);
    void disconnectFromPoint(PathPoint *point);

    void removeFromVectorPath();
    void removeApproximate();

    MovablePoint *getPointAtAbsPos(QPointF absPos, const CanvasMode &canvasMode);
    void rectPointsSelection(QRectF absRect, QList<MovablePoint *> *list);
    void updateStartCtrlPtVisibility();
    void updateEndCtrlPtVisibility();

    void setSeparatePathPoint(bool separatePathPoint);
    bool isSeparatePathPoint();

    void setCtrlsMode(CtrlsMode mode, bool saveUndoRedo = true);
    QPointF symmetricToAbsPos(QPointF absPosToMirror);
    QPointF symmetricToAbsPosNewLen(QPointF absPosToMirror, qreal newLen);
    void ctrlPointPosChanged(bool startPtChanged);
    void moveEndCtrlPtToAbsPos(QPointF endCtrlPt);
    void moveStartCtrlPtToAbsPos(QPointF startCtrlPt);
    void moveEndCtrlPtToRelPos(QPointF endCtrlPt);
    void moveStartCtrlPtToRelPos(QPointF startCtrlPt);
    void setCtrlPtEnabled(bool enabled, bool isStartPt, bool saveUndoRedo = true);
    VectorPath *getParentPath();

    void saveToSql(QSqlQuery *query, int boundingBoxId);

    void cancelTransform();

    void setEndCtrlPtEnabled(bool enabled);
    void setStartCtrlPtEnabled(bool enabled);

    bool isEndCtrlPtEnabled();
    bool isStartCtrlPtEnabled();

    void setPosAnimatorUpdater(AnimatorUpdater *updater);

    void updateAfterFrameChanged(int frame);

    PathPointAnimators *getPathPointAnimatorsPtr();
    void setPointId(int idT);
    int getPointId();

    CtrlsMode getCurrentCtrlsMode();

    PathPointValues getPointValues() const;
    void savePointValuesToShapeValues(VectorPathShape *shape);

    bool isNeighbourSelected();
    void moveByAbs(QPointF absTranslatione);
    void loadFromSql(int pathPointId, int movablePointId);
    PathPointValues getShapesInfluencedPointValues() const;
    void removeShapeValues(VectorPathShape *shape);

    void setPointValues(const PathPointValues &values);

    void editShape(VectorPathShape *shape);
    void finishEditingShape(VectorPathShape *shape);
    void cancelEditingShape();
    void addShapeValues(VectorPathShape *shape);
    void saveInitialPointValuesToShapeValues(VectorPathShape *shape);

    void makeDuplicate(MovablePoint *targetPoint);
    void duplicateCtrlPointsFrom(CtrlPoint *endPt,
                                 CtrlPoint *startPt);
    void reversePointsDirection();
    PathPoint *getConnectedSeparatePathPoint();

    virtual void drawHovered(QPainter *p) {
        p->setBrush(Qt::NoBrush);
        p->setPen(QPen(Qt::red, 2.));
        p->drawEllipse(getAbsolutePos(),
                       mRadius - 2, mRadius - 2);
    }
private:
    QList<PointShapeValues*> mShapeValues;

    bool mEditingShape = false;
    PathPointValues mBasisShapeSavedValues;

    int mPointId;
    PathPointAnimators mPathPointAnimators;

    VectorPath *mVectorPath;
    CtrlsMode mCtrlsMode = CtrlsMode::CTRLS_CORNER;

    bool mSeparatePathPoint = false;
    PathPoint *mNextPoint = NULL;
    PathPoint *mPreviousPoint = NULL;
    bool mStartCtrlPtEnabled = false;
    CtrlPoint *mStartCtrlPt;
    bool mEndCtrlPtEnabled = false;
    CtrlPoint *mEndCtrlPt;
    void ctrlPointPosChanged(CtrlPoint *pointChanged, CtrlPoint *pointToUpdate);
};

#endif // PATHPOINT_H
