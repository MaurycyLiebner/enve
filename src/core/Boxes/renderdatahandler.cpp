#include "renderdatahandler.h"

bool RenderDataHandler::removeItem(const stdsptr<BoxRenderData>& item) {
    return removeItemAtRelFrame(item->fRelFrame);
}

bool RenderDataHandler::removeItemAtRelFrame(const qreal frame) {
    return mFrameToData.erase(frameToKey(frame));
}

BoxRenderData *RenderDataHandler::getItemAtRelFrame(const qreal frame) const {
    const auto it = mFrameToData.find(frameToKey(frame));
    if(it == mFrameToData.end()) return nullptr;
    return it->second.get();
}

void RenderDataHandler::addItemAtRelFrame(
        const stdsptr<BoxRenderData>& item) {
    const int key = frameToKey(item->fRelFrame);
    mFrameToData.erase(key);
    mFrameToData.insert({key, item});
}
