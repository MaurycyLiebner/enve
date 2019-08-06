#ifndef CANVASRENDERDATA_H
#define CANVASRENDERDATA_H
#include "layerboxrenderdata.h"
struct CanvasRenderData : public ContainerBoxRenderData {
    CanvasRenderData(BoundingBox * const parentBoxT);

    int fCanvasWidth;
    int fCanvasHeight;
    SkColor fBgColor;

    SkColor eraseColor() const { return fBgColor; }
protected:
    void updateGlobalRect();
    void updateRelBoundingRect();
};

#endif // CANVASRENDERDATA_H
