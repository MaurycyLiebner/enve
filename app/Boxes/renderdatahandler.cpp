#include "renderdatahandler.h"

bool RenderDataHandler::removeItem(const stdsptr<BoundingBoxRenderData>& item) {
    return removeItemAtRelFrame(item->fRelFrame);
}

bool RenderDataHandler::removeItemAtRelFrame(const int frame) {
    return mFrameToData.erase(frame);
}

BoundingBoxRenderData *RenderDataHandler::getItemAtRelFrame(const int frame) {
    const auto it = mFrameToData.find(frame);
    if(it == mFrameToData.end()) return nullptr;
    return it->second.get();
}

void RenderDataHandler::addItemAtRelFrame(
        const stdsptr<BoundingBoxRenderData>& item) {
    mFrameToData.insert({item->fRelFrame, item});
    item->fRefInParent = true;
}
