#ifndef RENDERDATAHANDLER_H
#define RENDERDATAHANDLER_H
#include "boxrenderdata.h"
#include <map>

class RenderDataHandler {
public:
    void clear() { mFrameToData.clear(); }

    bool removeItem(const stdsptr<BoxRenderData> &item);
    bool removeItemAtRelFrame(const int frame);
    BoxRenderData *getItemAtRelFrame(const int frame) const;
    void addItemAtRelFrame(const stdsptr<BoxRenderData> &item);
protected:
    std::map<int, stdsptr<BoxRenderData>> mFrameToData;
};

#endif // RENDERDATAHANDLER_H
