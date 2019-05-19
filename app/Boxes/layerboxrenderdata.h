#ifndef LAYERBOXRENDERDATA_H
#define LAYERBOXRENDERDATA_H
#include "boundingboxrenderdata.h"

struct GroupBoxRenderData : public BoundingBoxRenderData {
    friend class StdSelfRef;
    QList<stdsptr<BoundingBoxRenderData>> fChildrenRenderData;
protected:
    GroupBoxRenderData(BoundingBox * const parentBoxT);

    void drawSk(SkCanvas * const canvas) {
        Q_UNUSED(canvas);
    }
    void transformRenderCanvas(SkCanvas& canvas) const final;
    void updateRelBoundingRect();
};

struct LayerBoxRenderData : public GroupBoxRenderData {
    friend class StdSelfRef;
protected:
    LayerBoxRenderData(BoundingBox * const parentBoxT);

    void drawSk(SkCanvas * const canvas);
};

#endif // LAYERBOXRENDERDATA_H
