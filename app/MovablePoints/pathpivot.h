#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "MovablePoints/nonanimatedmovablepoint.h"
#include "smartPointers/sharedpointerdefs.h"

class Canvas;

enum CanvasMode : short;

class PathPivot : public NonAnimatedMovablePoint {
    friend class StdSelfRef;
protected:
    PathPivot(const Canvas*  const parent);
public:
    void drawSk(SkCanvas * const canvas, const CanvasMode &mode,
                const SkScalar &invScale, const bool &keyOnCurrent);
    bool isVisible(const CanvasMode& mode) const;
protected:
    const Canvas * const mCanvas = nullptr;
};

#endif // PATHPIVOT_H
