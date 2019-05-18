#ifndef BOXESGROUPRENDERDATA_H
#define BOXESGROUPRENDERDATA_H
#include "boundingboxrenderdata.h"

struct BoxesGroupRenderData : public BoundingBoxRenderData {
    friend class StdSelfRef;
    QList<stdsptr<BoundingBoxRenderData>> fChildrenRenderData;
    bool fPaintOnImage;

    void processTask();

    void copyFrom(BoundingBoxRenderData *src);
protected:
    BoxesGroupRenderData(BoundingBox * const parentBoxT);

    void drawSk(SkCanvas * const canvas);
    void drawRenderedImageForParent(SkCanvas * const canvas);
    void transformRenderCanvas(SkCanvas& canvas) const final;

    void updateRelBoundingRect();
};

#endif // BOXESGROUPRENDERDATA_H
