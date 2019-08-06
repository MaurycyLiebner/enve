#ifndef LINKCANVASRENDERDATA_H
#define LINKCANVASRENDERDATA_H
#include "canvasrenderdata.h"
struct LinkCanvasRenderData : public CanvasRenderData {
    LinkCanvasRenderData(BoundingBox * const parentBoxT) :
        CanvasRenderData(parentBoxT) {}

    bool fClipToCanvas = false;
protected:
    void updateRelBoundingRect() {
        if(fClipToCanvas) CanvasRenderData::updateRelBoundingRect();
        else ContainerBoxRenderData::updateRelBoundingRect();
    }

    void updateGlobalRect() {
        if(fClipToCanvas) CanvasRenderData::updateGlobalRect();
        else ContainerBoxRenderData::updateGlobalRect();
    }

    SkColor eraseColor() const {
        if(fClipToCanvas) return fBgColor;
        else return SK_ColorTRANSPARENT;
    }
};

#endif // LINKCANVASRENDERDATA_H
