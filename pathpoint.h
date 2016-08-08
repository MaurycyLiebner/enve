#ifndef PATHPOINT_H
#define PATHPOINT_H
#include <QPointF>
#include "valueanimators.h"
#include <QPainter>


class VectorPath;

class PathPoint
{
public:
    PathPoint(QPointF absPos, VectorPath *vectorPath);

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

    void setNextPoint(PathPoint *mNextPoint);
    void setPreviousPoint(PathPoint *mPreviousPoint);
    bool hasNextPoint();
    bool hasPreviousPoint();
    void setPointAsNextOrPrevious(PathPoint *pointToSet);

    VectorPath *getParentPath();

    PathPoint *addPoint(QPointF absPos);
    bool isContainedInRect(QRectF absRect);
    void moveBy(QPointF absTranslation);

    void select();
    void deselect();

    bool isSelected();
private:
    bool mSelected = false;
    static const qreal RADIUS;
    PathPoint *mNextPoint = NULL;
    PathPoint *mPreviousPoint = NULL;
    QPointFAnimator mRelativePos;
    QPointFAnimator mStartCtrlPt;
    QPointFAnimator mEndCtrlPt;
    VectorPath *mVectorPath = NULL;
};

#endif // PATHPOINT_H
