#ifndef CONTAINERBOXRENDERDATA_H
#define CONTAINERBOXRENDERDATA_H
#include "boundingboxrenderdata.h"

struct ContainerBoxRenderData : public BoundingBoxRenderData {
    friend class StdSelfRef;
    QList<stdsptr<BoundingBoxRenderData>> fChildrenRenderData;
protected:
    ContainerBoxRenderData(BoundingBox * const parentBoxT);

    void drawSk(SkCanvas * const canvas);
    void transformRenderCanvas(SkCanvas& canvas) const final;
    void updateRelBoundingRect();
};

#endif // CONTAINERBOXRENDERDATA_H
