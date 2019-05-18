#ifndef BOXESGROUPRENDERDATA_H
#define BOXESGROUPRENDERDATA_H
#include "boundingboxrenderdata.h"

struct LayerBoxRenderData : public BoundingBoxRenderData {
    friend class StdSelfRef;
    QList<stdsptr<BoundingBoxRenderData>> fChildrenRenderData;
protected:
    LayerBoxRenderData(BoundingBox * const parentBoxT);

    void drawSk(SkCanvas * const canvas);
    void transformRenderCanvas(SkCanvas& canvas) const final;
    void updateRelBoundingRect();
};

#endif // BOXESGROUPRENDERDATA_H
