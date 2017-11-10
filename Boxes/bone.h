#ifndef BONE_H
#define BONE_H
#include "boundingbox.h"

class Bone : public BoundingBox {
public:
    Bone();
    void drawPixmapSk(SkCanvas *canvas);
    void drawPixmapSk(SkCanvas *canvas, SkPaint *paint);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
protected:
    NonAnimatedMovablePoint *mRootPt;
    NonAnimatedMovablePoint *mEndPt;
};

#endif // BONE_H
