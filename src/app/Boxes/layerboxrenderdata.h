#ifndef CONTAINERBOXRENDERDATA_H
#define CONTAINERBOXRENDERDATA_H
#include "boxrenderdata.h"

struct ContainerBoxRenderData : public BoxRenderData {
    friend class StdSelfRef;
    QList<stdsptr<BoxRenderData>> fChildrenRenderData;
protected:
    ContainerBoxRenderData(BoundingBox * const parentBoxT);

    void drawSk(SkCanvas * const canvas);
    void transformRenderCanvas(SkCanvas& canvas) const final;
    void updateRelBoundingRect();
};

#endif // CONTAINERBOXRENDERDATA_H
