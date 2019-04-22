#ifndef CANVASRENDERDATA_H
#define CANVASRENDERDATA_H
#include "boxesgrouprenderdata.h"
struct CanvasRenderData : public BoxesGroupRenderData {
    CanvasRenderData(BoundingBox * const parentBoxT);
    void processTask();
    qreal fCanvasWidth;
    qreal fCanvasHeight;
    SkColor fBgColor;
protected:
    void drawSk(SkCanvas * const canvas);
    void updateRelBoundingRect();
};

#endif // CANVASRENDERDATA_H
