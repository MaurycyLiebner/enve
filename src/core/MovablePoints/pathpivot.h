#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "MovablePoints/nonanimatedmovablepoint.h"
#include "smartPointers/sharedpointerdefs.h"

class Canvas;

enum CanvasMode : short;

class PathPivot : public NonAnimatedMovablePoint {
    friend class StdSelfRef;
protected:
    PathPivot(const Canvas* const parent);
public:
    void drawSk(SkCanvas * const canvas, const CanvasMode mode,
                const float invScale, const bool keyOnCurrent);
    void drawTransforming(SkCanvas * const canvas,
                          const CanvasMode mode,
                          const float invScale,
                          const float interval);
    bool isVisible(const CanvasMode mode) const;

    void setMousePos(const QPointF& pos) { mMousePos = pos; }
private:
    const Canvas * const mCanvas = nullptr;
    QPointF mMousePos;
};

#endif // PATHPIVOT_H
