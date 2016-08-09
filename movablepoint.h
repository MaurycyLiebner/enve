#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "valueanimators.h"
#include "connectedtomainwindow.h"

class VectorPath;
class PathPoint;

class MovablePoint : public ConnectedToMainWindow
{
public:
    MovablePoint(QPointF absPos, VectorPath *vectorPath, qreal radius = 5.f);

    void startTransform();
    void finishTransform();

    void setRelativePos(QPointF pos);
    QPointF getRelativePos();

    QPointF getAbsolutePos();

    virtual void draw(QPainter *p);

    bool isPointAt(QPointF absPoint);
    void setAbsolutePos(QPointF pos);

    VectorPath *getParentPath();

    bool isContainedInRect(QRectF absRect);
    virtual void moveBy(QPointF absTranslation);

    void select();
    void deselect();

    bool isSelected();

    virtual void remove();
protected:
    bool mSelected = false;
    qreal mRadius;
    QPointFAnimator mRelativePos;
    QPointF mSavedAbsPos;
    VectorPath *mVectorPath = NULL;
};

#endif // MOVABLEPOINT_H
