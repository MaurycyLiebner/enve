#ifndef PATHPOINT_H
#define PATHPOINT_H
#include <QPointF>
#include "valueanimators.h"
#include <QPainter>
#include "connectedtomainwindow.h"

class UndoRedoStack;

class VectorPath;

class PathPoint : public ConnectedToMainWindow
{
public:
    PathPoint(QPointF absPos, VectorPath *vectorPath);

    void startTransform();
    void finishTransform();

    void setRelativePos(QPointF pos);
    QPointF getRelativePos();

    QPointF getAbsolutePos();

    void setStartCtrlPt(QPointF startCtrlPt);
    QPointF getStartCtrlPt();

    void setEndCtrlPt(QPointF endCtrlPt);
    QPointF getEndCtrlPt();

    void draw(QPainter *p);

    PathPoint *getNextPoint();
    PathPoint *getPreviousPoint();

    bool isEndPoint();

    bool isPointAt(QPointF absPoint);
    void setAbsolutePos(QPointF pos);

    void setPointAsPrevious(PathPoint *pointToSet);
    void setPointAsNext(PathPoint *pointToSet);
    void setNextPoint(PathPoint *mNextPoint, bool saveUndoRedo = true);
    void setPreviousPoint(PathPoint *mPreviousPoint, bool saveUndoRedo = true);

    bool hasNextPoint();
    bool hasPreviousPoint();

    VectorPath *getParentPath();

    PathPoint *addPoint(QPointF absPos);
    bool isContainedInRect(QRectF absRect);
    void moveBy(QPointF absTranslation);

    void select();
    void deselect();

    bool isSelected();

    void connectToPoint(PathPoint *point);
    void disconnectFromPoint(PathPoint *point);

    void remove();
private:
    bool mSelected = false;
    static const qreal RADIUS;
    PathPoint *mNextPoint = NULL;
    PathPoint *mPreviousPoint = NULL;
    QPointFAnimator mRelativePos;
    QPointF mSavedAbsPos;
    QPointFAnimator mStartCtrlPt;
    QPointFAnimator mEndCtrlPt;
    VectorPath *mVectorPath = NULL;
};

#endif // PATHPOINT_H
