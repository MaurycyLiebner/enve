#ifndef BOXESGROUPRENDERDATA_H
#define BOXESGROUPRENDERDATA_H
#include "boundingboxrenderdata.h"

struct BoxesGroupRenderData : public BoundingBoxRenderData {
    friend class StdSelfRef;
    QList<stdsptr<BoundingBoxRenderData>> fChildrenRenderData;
protected:
    BoxesGroupRenderData(BoundingBox * const parentBoxT);

    void drawSk(SkCanvas * const canvas);

    void transformRenderCanvas(SkCanvas& canvas) const;

    void updateRelBoundingRect();
};

#endif // BOXESGROUPRENDERDATA_H
