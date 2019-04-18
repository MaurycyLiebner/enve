#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "MovablePoints/nonanimatedmovablepoint.h"
#include "smartPointers/sharedpointerdefs.h"

class Canvas;

enum CanvasMode : short;

class PathPivot : public NonAnimatedMovablePoint {
    friend class StdSelfRef;
protected:
    PathPivot(Canvas*  const parent);
public:
    void drawSk(SkCanvas * const canvas, const CanvasMode &mode,
                const SkScalar &invScale, const bool &keyOnCurrent);
    bool isVisible(const CanvasMode& mode) const;
    //void setRelativePos(const QPointF &relPos);

    bool isRotating();
    bool isScaling();
    bool handleMousePress(const QPointF &absPos,
                          const CanvasMode &mode,
                          const qreal &invScale);
    bool handleMouseRelease();
    bool handleMouseMove(const QPointF &moveDestAbs,
                         const QPointF &pressPos,
                         const bool &xOnly,
                         const bool &yOnly,
                         const bool &inputTransformationEnabled,
                         const qreal &inputTransformationValue,
                         const bool &startTransform,
                         const CanvasMode &mode);
    void startRotating();
    void startScaling();
protected:
    bool mRotating = false;
    bool mScaling = false;
    int mRotHalfCycles = 0;
    qreal mLastDRot = 0;
    Canvas* mCanvas = nullptr;
};

#endif // PATHPIVOT_H
