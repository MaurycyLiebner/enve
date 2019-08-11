#ifndef PATHPIVOT_H
#define PATHPIVOT_H
#include "MovablePoints/nonanimatedmovablepoint.h"
#include "smartPointers/ememory.h"

class Canvas;

enum class CanvasMode : short;

class PathPivot : public NonAnimatedMovablePoint {
    e_OBJECT
protected:
    PathPivot(const Canvas* const parent);
public:
    void drawSk(SkCanvas * const canvas, const CanvasMode mode,
                const float invScale, const bool keyOnCurrent,
                const bool ctrlPressed);
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
