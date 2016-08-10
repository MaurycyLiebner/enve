#ifndef PATHPOINT_H
#define PATHPOINT_H
#include "movablepoint.h"

class UndoRedoStack;

class VectorPath;

enum CanvasMode : short;

enum CtrlsMode {
    CTRLS_SMOOTH,
    CTRLS_SYMMETRIC,
    CTRLS_CORNER
};

class PathPoint : public MovablePoint
{
public:
    PathPoint(QPointF absPos, VectorPath *vectorPath);
    PathPoint(QPointF absPos, QPointF startCtrlAbsPos, QPointF endCtrlAbsPos, VectorPath *vectorPath);

    void startTransform();
    void finishTransform();

    void moveBy(QPointF absTranslation);
    void moveToAbs(QPointF absPos);

    void setStartCtrlPtAbsPos(QPointF startCtrlPt);
    QPointF getStartCtrlPtAbsPos();
    QPointF getStartCtrlPtRelativePos();
    MovablePoint *getStartCtrlPt();

    void setEndCtrlPtAbsPos(QPointF endCtrlPt);
    QPointF getEndCtrlPtAbsPos();
    QPointF getEndCtrlPtRelativePos();
    MovablePoint *getEndCtrlPt();

    void draw(QPainter *p, CanvasMode mode);

    PathPoint *getNextPoint();
    PathPoint *getPreviousPoint();

    bool isEndPoint();

    void setPointAsPrevious(PathPoint *pointToSet);
    void setPointAsNext(PathPoint *pointToSet);
    void setNextPoint(PathPoint *mNextPoint, bool saveUndoRedo = true);
    void setPreviousPoint(PathPoint *mPreviousPoint, bool saveUndoRedo = true);

    bool hasNextPoint();
    bool hasPreviousPoint();

    PathPoint *addPoint(QPointF absPos);
    PathPoint *addPoint(PathPoint *pointToAdd);

    void connectToPoint(PathPoint *point);
    void disconnectFromPoint(PathPoint *point);

    void remove();

    MovablePoint *getPointAtAbsPos(QPointF absPos, CanvasMode canvasMode);
    void rectPointsSelection(QRectF absRect, QList<MovablePoint *> *list);
    void updateStartCtrlPtVisibility();
    void updateEndCtrlPtVisibility();

    void setEndCtrlPtEnabled(bool enabled);
    void setStartCtrlPtEnabled(bool enabled);

    void setSeparatePathPoint(bool separatePathPoint);
    bool isSeparatePathPoint();

    void setCtrlsMode(CtrlsMode mode);
    QPointF symmetricToAbsPos(QPointF absPosToMirror);
private:
    CtrlsMode mCtrlsMode = CtrlsMode::CTRLS_SYMMETRIC;

    bool mSeparatePathPoint = false;
    PathPoint *mNextPoint = NULL;
    PathPoint *mPreviousPoint = NULL;
    bool mStartCtrlPtEnabled = true;
    MovablePoint *mStartCtrlPt;
    bool mEndCtrlPtEnabled = true;
    MovablePoint *mEndCtrlPt;
};

#endif // PATHPOINT_H
