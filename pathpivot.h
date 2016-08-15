#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "movablepoint.h"

class Canvas;

class PathPivot : public MovablePoint
{
public:
    PathPivot(Canvas *parent);

    void draw(QPainter *p);
    void setRelativePos(QPointF relPos, bool saveUndoRedo);

    bool isRotating();
    bool handleMousePress(QPointF absPressPos);
    bool handleMouseRelease();
    bool handleMouseMove(QPointF moveDestAbs, QPointF moveBy, bool startTransform);
    void updateRotationMappedPath();
    void finishTransform();
private:
    Canvas *mCanvas;
    bool mRotating = false;
    bool isRotationPathAt(QPointF absPos);

    QPainterPath mRotationPath;
    QPainterPath mMappedRotationPath;
};

#endif // PATHPIVOT_H
