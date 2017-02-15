#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "movablepoint.h"

class Canvas;

enum CanvasMode : short;

class PathPivot : public MovablePoint
{
public:
    PathPivot(Canvas *parent);

    void draw(QPainter *p);
    void setRelativePos(QPointF relPos, bool saveUndoRedo);

    bool isRotating();
    bool isScaling();
    bool handleMousePress(QPointF absPressPos);
    bool handleMouseRelease();
    bool handleMouseMove(QPointF moveDestAbs, QPointF pressPos,
                         bool xOnly, bool yOnly,
                         bool inputTransformationEnabled,
                         qreal inputTransformationValue,
                         bool startTransform, const CanvasMode &mode);
//    void updateRotationMappedPath();
    void finishTransform();
    void startRotating();
    void startScaling();
    void startTransform();
private:
    qreal mLastDRot = 0.;
    int mRotHalfCycles = 0;
    Canvas *mCanvas;
    bool mRotating = false;
    bool mScaling = false;
//    bool isRotationPathAt(QPointF absPos);

//    QPainterPath mRotationPath;
//    QPainterPath mMappedRotationPath;
};

#endif // PATHPIVOT_H
