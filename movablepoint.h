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
    MovablePoint(QPointF absPos, VectorPath *vectorPath, qreal radius = 7.5f);

    virtual void startTransform();
    virtual void finishTransform();

    void setRelativePos(QPointF pos);
    QPointF getRelativePos();

    QPointF getAbsolutePos();

    virtual void draw(QPainter *p);

    bool isPointAt(QPointF absPoint);
    void setAbsolutePos(QPointF pos);

    VectorPath *getParentPath();

    bool isContainedInRect(QRectF absRect);
    virtual void moveBy(QPointF absTranslation);
    virtual void moveToAbs(QPointF absPos);

    void select();
    void deselect();

    bool isSelected();

    virtual void remove();

    void hide();
    void show();
    bool isHidden();
    bool isVisible();
    void setVisible(bool bT);
protected:
    bool mHidden = false;
    bool mSelected = false;
    qreal mRadius;
    QPointFAnimator mRelativePos;
    QPointF mSavedAbsPos;
    VectorPath *mVectorPath = NULL;
};

#endif // MOVABLEPOINT_H
