#include "renderdatahandler.h"

bool RenderDataHandler::removeItem(BoundingBoxRenderData *item) {
    return removeItemAtRelFrame(item->relFrame);
}

bool RenderDataHandler::removeItemAtRelFrame(const int &frame) {
    int id;
    if(getItemIdAtRelFrame(frame, &id)) {
        mItems.removeAt(id);
        return true;
    }
    return false;
}

BoundingBoxRenderData *RenderDataHandler::getItemAtRelFrame(const int &frame) {
    int id;
    if(getItemIdAtRelFrame(frame, &id)) {
        return mItems.at(id);
    }
    return BoundingBoxRenderDataSPtr();
}

void RenderDataHandler::addItemAtRelFrame(BoundingBoxRenderData *item) {
    int itemId = getItemInsertIdAtRelFrame(item->relFrame);
    mItems.insert(itemId, item);
}

int RenderDataHandler::getItemInsertIdAtRelFrame(const int &relFrame) {
    int minId = 0;
    int maxId = mItems.count();

    while(minId < maxId) {
        int guess = (minId + maxId)/2;
        BoundingBoxRenderDataSPtr item = mItems.at(guess);
        int contFrame = item->relFrame;
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
        int guess = (minId + maxId)/2;
        BoundingBoxRenderDataSPtr item = mItems.at(guess);
        if(item->relFrame == relFrame) {
            *id = guess;
            return true;
        }
        int contFrame = item->relFrame;
        if(contFrame > relFrame) {
            if(maxId == guess) {
                *id = minId;
                return mItems.at(minId)->relFrame == relFrame;
            } else {
                maxId = guess;
            }
        } else if(contFrame < relFrame) {
            if(minId == guess) {
                *id = maxId;
                return mItems.at(maxId)->relFrame == relFrame;
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
