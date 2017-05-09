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
class VectorPathEdge;

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

class PathPointAnimators : public ComplexAnimator {
public:
    PathPointAnimators();

    void setAllVars(PathPoint *parentPathPointT,
                    QPointFAnimator *endPosAnimatorT,
                    QPointFAnimator *startPosAnimatorT);

    bool isOfPathPoint(PathPoint *checkPoint) {
        return mParentPathPoint == checkPoint;
    }

    int prp_saveToSql(QSqlQuery *, const int &parentId) {

    }

    void prp_loadFromSql(const int &identifyingId) {

    }

    void makeDuplicate(Property *) {

    }

    Property *makeDuplicate() {

    }
private:
    PathPoint *mParentPathPoint;
    QPointFAnimator *mEndPosAnimator;
    QPointFAnimator *mStartPosAnimator;
}; 

class SinglePathAnimator;

class PathPoint : public MovablePoint
{
public:
    PathPoint(SinglePathAnimator *parentAnimator);

    ~PathPoint();

    void applyTransform(const QMatrix &transform);

    void startTransform();
    void finishTransform();

    void moveByRel(const QPointF &relTranslation);

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

    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                                   const CanvasMode &canvasMode,
                                   const qreal &canvasScaleInv);
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
    SinglePathAnimator *getParentPath();

    int prp_saveToSql(QSqlQuery *query, const int &boundingBoxId);

    void cancelTransform();

    void setEndCtrlPtEnabled(bool enabled, bool saveUndoRedo = true);
    void setStartCtrlPtEnabled(bool enabled, bool saveUndoRedo = true);

    bool isEndCtrlPtEnabled();
    bool isStartCtrlPtEnabled();

    void setPosAnimatorUpdater(AnimatorUpdater *updater);

    void updateAfterFrameChanged(const int &frame);

    PathPointAnimators *getPathPointAnimatorsPtr();
    void setPointId(int idT);
    int getPointId();

    CtrlsMode getCurrentCtrlsMode();

    PathPointValues getPointValues() const;

    bool isNeighbourSelected();
    void moveByAbs(const QPointF &absTranslatione);
    void prp_loadFromSql(const int &movablePointId);

    void setPointValues(const PathPointValues &values);


    void makeDuplicate(MovablePoint *targetPoint);
    void duplicateCtrlPointsFrom(CtrlPoint *endPt,
                                 CtrlPoint *startPt);
    void reversePointsDirection();
    PathPoint *getConnectedSeparatePathPoint();

    virtual void drawHovered(QPainter *p) {
        p->setBrush(Qt::NoBrush);
        QPen pen = QPen(Qt::red, 2.);
        pen.setCosmetic(true);
        p->setPen(pen);
        drawCosmeticEllipse(p, getAbsolutePos(),
                            mRadius - 2., mRadius - 2.);
    }
    void setParentPath(SinglePathAnimator *path);
    void reversePointsDirectionStartingFromThis(
            const bool &saveUndoRedo = true);
    void reversePointsDirectionReverse();

    void saveTransformPivotAbsPos(QPointF absPivot);
    void rotateRelativeToSavedPivot(const qreal &rot);

    VectorPathEdge *getNextEdge() {
        return mNextEdge.get();
    }
private:
    std::shared_ptr<VectorPathEdge> mNextEdge;

    int mPointId;
    QSharedPointer<PathPointAnimators> mPathPointAnimators =
            (new PathPointAnimators())->ref<PathPointAnimators>();

    SinglePathAnimator *mParentPath;
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
