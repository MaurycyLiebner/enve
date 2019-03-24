#ifndef LINKCANVASRENDERDATA_H
#define LINKCANVASRENDERDATA_H
#include "canvasrenderdata.h"
struct LinkCanvasRenderData : public CanvasRenderData {
    friend class StdSelfRef;

    void updateRelBoundingRect() {
        if(fClipToCanvas) CanvasRenderData::updateRelBoundingRect();
        else BoxesGroupRenderData::updateRelBoundingRect();
    }

    bool fClipToCanvas = false;
protected:
    LinkCanvasRenderData(BoundingBox * const parentBoxT) :
        CanvasRenderData(parentBoxT) {}

    void renderToImage();
};
#endif // LINKCANVASRENDERDATA_H
