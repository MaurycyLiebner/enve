#ifndef CANVASRENDERDATA_H
#define CANVASRENDERDATA_H
#include "layerboxrenderdata.h"
struct CanvasRenderData : public LayerBoxRenderData {
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
