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

struct PathPointAnimators {

    void setAllVars(PathPoint *parentPathPointT,
                       QPointFAnimator *endPosAnimatorT,
                       QPointFAnimator *startPosAnimatorT,
                       QPointFAnimator *pathPointPosAnimatorT) {
        parentPathPoint = parentPathPointT;
        endPosAnimator = endPosAnimatorT;
        startPosAnimator = startPosAnimatorT;
        pathPointPosAnimator = pathPointPosAnimatorT;
    }

    bool isOfPathPoint(PathPoint *checkPoint) {
        return parentPathPoint == checkPoint;
    }

    void updateKeysPath() {
        endPosAnimator->updateKeysPath();
        startPosAnimator->updateKeysPath();
        pathPointPosAnimator->updateKeysPath();
    }

    void sortKeys() {
        endPosAnimator->sortKeys();
        startPosAnimator->sortKeys();
        pathPointPosAnimator->sortKeys();
    }

    qreal getBoxesListHeight() {
        return endPosAnimator->getBoxesListHeight() +
               startPosAnimator->getBoxesListHeight() +
               pathPointPosAnimator->getBoxesListHeight();
    }

    void drawBoxesList(QPainter *p,
                       qreal drawX, qreal drawY,
                       qreal pixelsPerFrame,
                       int startFrame, int endFrame) {
        endPosAnimator->drawBoxesList(p, drawX, drawY,
                                      pixelsPerFrame, startFrame, endFrame);
        startPosAnimator->drawBoxesList(p, drawX, drawY,
                                      pixelsPerFrame, startFrame, endFrame);
        pathPointPosAnimator->drawBoxesList(p, drawX, drawY,
                                      pixelsPerFrame, startFrame, endFrame);
    }

    QrealKey *getKeyAtPos(qreal relX, qreal relY,
                          int minViewedFrame, qreal pixelsPerFrame) {
        qreal endHeight = endPosAnimator->getBoxesListHeight();
        if(relY <= endHeight) {
            return endPosAnimator->getKeyAtPos(relX, relY - endHeight,
                                               minViewedFrame, pixelsPerFrame);
        }
        relY -= endHeight;
        qreal startHeight = startPosAnimator->getBoxesListHeight();
        if(relY <= startHeight) {
            return startPosAnimator->getKeyAtPos(relX, relY - startHeight,
                                               minViewedFrame, pixelsPerFrame);
        }
        relY -= startHeight;
        qreal pointHeight = pathPointPosAnimator->getBoxesListHeight();
        if(relY <= pointHeight) {
            return pathPointPosAnimator->getKeyAtPos(relX, relY - pointHeight,
                                               minViewedFrame, pixelsPerFrame);
        }
        return NULL;
    }


    PathPoint *parentPathPoint;
    QPointFAnimator *endPosAnimator;
    QPointFAnimator *startPosAnimator;
    QPointFAnimator *pathPointPosAnimator;
};

class PathPoint : public MovablePoint
{
public:
    PathPoint(QPointF absPos, VectorPath *vectorPath);
    PathPoint(QPointF absPos, QPointF startCtrlAbsPos, QPointF endCtrlAbsPos,
              VectorPath *vectorPath);
    PathPoint(int movablePointId, int pathPointId, VectorPath *vectorPath);

    void startTransform();
    void finishTransform();

    void moveBy(QPointF absTranslation);
    void moveToAbs(QPointF absPos);

    QPointF getStartCtrlPtAbsPos();
    QPointF getStartCtrlPtValue();
    MovablePoint *getStartCtrlPt();

    QPointF getEndCtrlPtAbsPos();
    QPointF getEndCtrlPtValue();
    MovablePoint *getEndCtrlPt();

    void draw(QPainter *p, CanvasMode mode);

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

    void remove();

    MovablePoint *getPointAtAbsPos(QPointF absPos, CanvasMode canvasMode);
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

    void saveToSql(int vectorPathId);
    PathPoint(qreal relPosX, qreal relPosy,
              qreal startCtrlRelX, qreal startCtrlRelY,
              qreal endCtrlRelX, qreal endCtrlRelY, bool isFirst, int boneZ, VectorPath *vectorPath);

    void clearAll();
    void cancelTransform();
    void attachToBoneFromSqlZId();

    void setEndCtrlPtEnabled(bool enabled);
    void setStartCtrlPtEnabled(bool enabled);

    bool isEndCtrlPtEnabled();
    bool isStartCtrlPtEnabled();

    void setPosAnimatorUpdater(AnimatorUpdater *updater);

    void updateAfterFrameChanged(int frame);

    PathPointAnimators *getPathPointAnimatorsPtr();
private:
    PathPointAnimators mPathPointAnimators;

    VectorPath *mVectorPath;
    CtrlsMode mCtrlsMode = CtrlsMode::CTRLS_SYMMETRIC;

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
