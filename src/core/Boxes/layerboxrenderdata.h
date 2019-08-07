#ifndef CONTAINERBOXRENDERDATA_H
#define CONTAINERBOXRENDERDATA_H
#include "boxrenderdata.h"

struct ContainerBoxRenderData : public BoxRenderData {
    e_OBJECT
public:
    QList<stdsptr<BoxRenderData>> fChildrenRenderData;
    ContainerBoxRenderData(BoundingBox * const parentBoxT);
protected:
    void drawSk(SkCanvas * const canvas);
    void transformRenderCanvas(SkCanvas& canvas) const final;
    void updateRelBoundingRect();
};

#endif // CONTAINERBOXRENDERDATA_H
