#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "valueanimators.h"
#include "connectedtomainwindow.h"

class VectorPath;
class PathPoint;

enum MovablePointType {
    TYPE_PATH_POINT,
    TYPE_CTRL_POINT,
    TYPE_PIVOT_POINT
};

class MovablePoint : public ConnectedToMainWindow
{
public:
    MovablePoint(QPointF absPos, VectorPath *vectorPath, MovablePointType type, qreal radius = 7.5f);

    virtual void startTransform();
    virtual void finishTransform();


    QPointF getRelativePos();

    QPointF getAbsolutePos();

    virtual void draw(QPainter *p);

    bool isPointAt(QPointF absPoint);
    void setAbsolutePos(QPointF pos, bool saveUndoRedo = true);

    VectorPath *getParentPath();

    bool isContainedInRect(QRectF absRect);
    virtual void moveBy(QPointF absTranslatione);
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

    bool isPathPoint();
    bool isPivotPoint();
    bool isCtrlPoint();
protected:
    bool mTransformStarted = false;
    MovablePointType mType;
    bool mHidden = false;
    bool mSelected = false;
    qreal mRadius;
    QPointFAnimator mRelativePos;
    QPointF mSavedAbsPos;
    VectorPath *mVectorPath = NULL;
private:
    void setRelativePos(QPointF pos);
};

#endif // MOVABLEPOINT_H
