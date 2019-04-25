#include "renderdatahandler.h"

bool RenderDataHandler::removeItem(const stdsptr<BoundingBoxRenderData>& item) {
    return removeItemAtRelFrame(item->fRelFrame);
}

bool RenderDataHandler::removeItemAtRelFrame(const int &frame) {
    int id;
    if(getItemIdAtRelFrame(frame, &id)) {
        mItems.takeAt(id)->fRefInParent = false;
        return true;
    }
    return false;
}

BoundingBoxRenderData *RenderDataHandler::getItemAtRelFrame(const int &frame) {
    int id;
    if(getItemIdAtRelFrame(frame, &id)) {
        const auto item = mItems.at(id).get();
        return item;
    }
    return nullptr;
}

void RenderDataHandler::addItemAtRelFrame(
        const stdsptr<BoundingBoxRenderData>& item) {
    const int itemId = getItemInsertIdAtRelFrame(item->fRelFrame);
    mItems.insert(itemId, item);
    item->fRefInParent = true;
}

int RenderDataHandler::getItemInsertIdAtRelFrame(const int &relFrame) {
    int minId = 0;
    int maxId = mItems.count();

    while(minId < maxId) {
        const int guess = (minId + maxId)/2;
        stdsptr<BoundingBoxRenderData> item = mItems.at(guess);
        const int contFrame = item->fRelFrame;
        Q_ASSERT(contFrame != relFrame);
        if(contFrame > relFrame) {
            if(guess == maxId) {
                return minId;
            }
            maxId = guess;
        } else if(contFrame < relFrame) {
            if(guess == minId) {
                return maxId;
            }
            minId = guess;
        }
    }
    return 0;
}

bool RenderDataHandler::getItemIdAtRelFrame(const int &relFrame, int *id) {
    int minId = 0;
    int maxId = mItems.count() - 1;

    while(minId <= maxId) {
        const int guess = (minId + maxId)/2;
        stdsptr<BoundingBoxRenderData> item = mItems.at(guess);
        if(item->fRelFrame == relFrame) {
            *id = guess;
            return true;
        }
        const int contFrame = item->fRelFrame;
        if(contFrame > relFrame) {
            if(maxId == guess) {
                *id = minId;
                return mItems.at(minId)->fRelFrame == relFrame;
            } else {
                maxId = guess;
            }
        } else if(contFrame < relFrame) {
            if(minId == guess) {
                *id = maxId;
                return mItems.at(maxId)->fRelFrame == relFrame;
            } else {
                minId = guess;
            }
        } else {
            *id = guess;
            return true;
        }
    }
    return false;
}
