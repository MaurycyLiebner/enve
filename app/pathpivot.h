#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "movablepoint.h"
#include "sharedpointerdefs.h"

class Canvas;

enum CanvasMode : short;

class PathPivot : public NonAnimatedMovablePoint {
public:
    PathPivot(Canvas* parent);

    //void setRelativePos(const QPointF &relPos);

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
    void finishTransform();
    void startRotating();
    void startScaling();
    void drawSk(SkCanvas *canvas, const SkScalar &invScale);
protected:
    bool mRotating = false;
    bool mScaling = false;
    int mRotHalfCycles = 0;
    qreal mLastDRot = 0.;
    Canvas* mCanvas = nullptr;
};

#endif // PATHPIVOT_H
