#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "movablepoint.h"

class Canvas;

enum CanvasMode : short;

class PathPivot : public MovablePoint
{
public:
    PathPivot(Canvas *parent);

    void setRelativePos(const QPointF &relPos,
                        const bool &saveUndoRedo = true);

    bool isRotating();
    bool isScaling();
    bool handleMousePress(const QPointF &absPressPos,
                          const qreal &canvasInvScale);
    bool handleMouseRelease();
    bool handleMouseMove(const QPointF &moveDestAbs,
                         const QPointF &pressPos,
                         const bool &xOnly,
                         const bool &yOnly,
                         const bool &inputTransformationEnabled,
                         const qreal &inputTransformationValue,
                         const bool &startTransform,
                         const CanvasMode &mode);
//    void updateRotationMappedPath();
    void finishTransform();
    void startRotating();
    void startScaling();
    void startTransform();
    void drawSk(SkCanvas *canvas, const SkScalar &invScale);
protected:
    void drawOnAbsPos(QPainter *p, const QPointF &absPos);

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
