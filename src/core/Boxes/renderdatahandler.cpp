#include "renderdatahandler.h"

bool RenderDataHandler::removeItem(const stdsptr<BoxRenderData>& item) {
    return removeItemAtRelFrame(item->fRelFrame);
}

bool RenderDataHandler::removeItemAtRelFrame(const int frame) {
    return mFrameToData.erase(frame);
}

BoxRenderData *RenderDataHandler::getItemAtRelFrame(const int frame) const {
    const auto it = mFrameToData.find(frame);
    if(it == mFrameToData.end()) return nullptr;
    return it->second.get();
}

void RenderDataHandler::addItemAtRelFrame(
        const stdsptr<BoxRenderData>& item) {
    const auto ret = mFrameToData.insert({item->fRelFrame, item});
    if(!ret.second)
        RuntimeThrow("Item already present at rel frame" +
                     QString::number(item->fRelFrame));
    item->fRefInParent = true;
}
