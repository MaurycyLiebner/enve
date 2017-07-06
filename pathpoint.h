#ifndef PATHPOINT_H
#define PATHPOINT_H
#include <QSqlQuery>
#include "movablepoint.h"

class UndoRedoStack;

class VectorPath;
class SkCanvas;
class CtrlPoint;

class SingleVectorPathAnimator;

enum CanvasMode : short;
enum CtrlsMode : short;

class PathPoint;
class VectorPathEdge;

struct PathPointValues {
    PathPointValues(const QPointF &startPosT,
                    const QPointF &pointPosT,
                    const QPointF &endPosT) {
        startRelPos = startPosT;
        pointRelPos = pointPosT;
        endRelPos = endPosT;
    }

    PathPointValues() {}


    QPointF startRelPos;
    QPointF pointRelPos;
    QPointF endRelPos;

    PathPointValues &operator/=(const qreal &val) {
        qreal inv = 1.f/val;
        startRelPos *= inv;
        pointRelPos *= inv;
        endRelPos *= inv;
        return *this;
    }
    PathPointValues &operator*=(const qreal &val) {
        startRelPos *= val;
        pointRelPos *= val;
        endRelPos *= val;
        return *this;
    }
    PathPointValues &operator+=(const PathPointValues &ppv) {
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
private:
    PathPoint *mParentPathPoint;
    QPointFAnimator *mEndPosAnimator;
    QPointFAnimator *mStartPosAnimator;
}; 

class SinglePathAnimator;

class PathPoint : public MovablePoint
{
public:
    PathPoint(SingleVectorPathAnimator *parentAnimator);

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

    void drawSk(SkCanvas *canvas,
              const CanvasMode &mode,
              const SkScalar &invScale);

    PathPoint *getNextPoint();
    PathPoint *getPreviousPoint();

    bool isEndPoint();

    void setPointAsPrevious(PathPoint *pointToSet,
                            const bool &saveUndoRedo = true);
    void setPointAsNext(PathPoint *pointToSet,
                        const bool &saveUndoRedo = true);
    void setNextPoint(PathPoint *mNextPoint,
                      const bool &saveUndoRedo = true);
    void setPreviousPoint(PathPoint *mPreviousPoint,
                          const bool &saveUndoRedo = true);

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
    void rectPointsSelection(QRectF absRect,
                             QList<MovablePoint *> *list);
    void updateStartCtrlPtVisibility();
    void updateEndCtrlPtVisibility();

    void setSeparatePathPoint(const bool &separatePathPoint);
    bool isSeparatePathPoint();

    void setCtrlsMode(const CtrlsMode &mode,
                      const bool &saveUndoRedo = true);
    QPointF symmetricToAbsPos(QPointF absPosToMirror);
    QPointF symmetricToAbsPosNewLen(QPointF absPosToMirror,
                                    qreal newLen);
    void ctrlPointPosChanged(bool startPtChanged);
    void moveEndCtrlPtToAbsPos(QPointF endCtrlPt);
    void moveStartCtrlPtToAbsPos(QPointF startCtrlPt);
    void moveEndCtrlPtToRelPos(QPointF endCtrlPt);
    void moveStartCtrlPtToRelPos(QPointF startCtrlPt);
    void setCtrlPtEnabled(const bool &enabled,
                          const bool &isStartPt,
                          const bool &saveUndoRedo = true);
    SingleVectorPathAnimator *getParentPath();

    int saveToSql(QSqlQuery *query, const int &boundingBoxId);

    void cancelTransform();

    void setEndCtrlPtEnabled(const bool &enabled,
                             const bool &saveUndoRedo = true);
    void setStartCtrlPtEnabled(const bool &enabled,
                               const bool &saveUndoRedo = true);

    void resetEndCtrlPt();
    void resetStartCtrlPt();

    bool isEndCtrlPtEnabled();
    bool isStartCtrlPtEnabled();

    void setPosAnimatorUpdater(AnimatorUpdater *updater);

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

    void setParentPath(SingleVectorPathAnimator *path);
    void reversePointsDirectionStartingFromThis(
            const bool &saveUndoRedo = true);
    void reversePointsDirectionReverse();

    void saveTransformPivotAbsPos(const QPointF &absPivot);
    void rotateRelativeToSavedPivot(const qreal &rot);

    VectorPathEdge *getNextEdge() {
        return mNextEdge.get();
    }
    QPointF getEndCtrlPtValueAtRelFrame(const int &relFrame) const;
    QPointF getStartCtrlPtValueAtRelFrame(const int &relFrame) const;
    PathPointValues getPointValuesAtRelFrame(const int &relFrame);
private:
    std::shared_ptr<VectorPathEdge> mNextEdge;

    int mPointId;
    QSharedPointer<PathPointAnimators> mPathPointAnimators =
            (new PathPointAnimators())->ref<PathPointAnimators>();

    SingleVectorPathAnimator *mParentPath;
    CtrlsMode mCtrlsMode;

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
