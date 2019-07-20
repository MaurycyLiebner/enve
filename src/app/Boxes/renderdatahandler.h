#ifndef RENDERDATAHANDLER_H
#define RENDERDATAHANDLER_H
#include "boundingboxrenderdata.h"
#include <map>

class RenderDataHandler {
public:
    void clear() { mFrameToData.clear(); }

    bool removeItem(const stdsptr<BoundingBoxRenderData> &item);
    bool removeItemAtRelFrame(const int frame);
    BoundingBoxRenderData *getItemAtRelFrame(const int frame) const;
    void addItemAtRelFrame(const stdsptr<BoundingBoxRenderData> &item);
protected:
    std::map<int, stdsptr<BoundingBoxRenderData>> mFrameToData;
};

#endif // RENDERDATAHANDLER_H
