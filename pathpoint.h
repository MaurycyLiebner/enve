#ifndef PATHPOINT_H
#define PATHPOINT_H
#include "movablepoint.h"

class UndoRedoStack;

class VectorPath;

enum CanvasMode : short;

class PathPoint : public MovablePoint
{
public:
    PathPoint(QPointF absPos, VectorPath *vectorPath);

    void setStartCtrlPtRelativePos(QPointF startCtrlPt);
    QPointF getStartCtrlPtRelativePos();
    MovablePoint *getStartCtrlPt();

    void setEndCtrlPtRelativePos(QPointF endCtrlPt);
    QPointF getEndCtrlPtRelativePos();
    MovablePoint *getEndCtrlPt();

    void draw(QPainter *p);

    PathPoint *getNextPoint();
    PathPoint *getPreviousPoint();

    bool isEndPoint();

    void setAbsolutePos(QPointF pos);

    void setPointAsPrevious(PathPoint *pointToSet);
    void setPointAsNext(PathPoint *pointToSet);
    void setNextPoint(PathPoint *mNextPoint, bool saveUndoRedo = true);
    void setPreviousPoint(PathPoint *mPreviousPoint, bool saveUndoRedo = true);

    bool hasNextPoint();
    bool hasPreviousPoint();

    PathPoint *addPoint(QPointF absPos);

    void connectToPoint(PathPoint *point);
    void disconnectFromPoint(PathPoint *point);

    void remove();

    void moveBy(QPointF absTranslation);

    MovablePoint *getPointAtAbsPos(QPointF absPos, CanvasMode canvasMode);
private:
    PathPoint *mNextPoint = NULL;
    PathPoint *mPreviousPoint = NULL;
    MovablePoint *mStartCtrlPt;
    MovablePoint *mEndCtrlPt;
};

#endif // PATHPOINT_H
