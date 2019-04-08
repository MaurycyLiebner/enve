#include "filecachehandler.h"
#include "Boxes/boundingbox.h"
#include "filesourcescache.h"

FileCacheHandler::FileCacheHandler() {}

void FileCacheHandler::clearCache() {
    const QFile file(mFilePath);
    mFileMissing = !file.exists();
    for(const auto &boxPtr : mDependentBoxes) {
        if(boxPtr) boxPtr->reloadCacheHandler();
    }
}

void FileCacheHandler::addDependentBox(BoundingBox * const dependent) {
    mDependentBoxes << dependent;
}

void FileCacheHandler::removeDependentBox(BoundingBox * const dependent) {
    for(int i = 0; i < mDependentBoxes.count(); i++) {
        const auto &boxPtr = mDependentBoxes.at(i);
        if(boxPtr == dependent) {
            mDependentBoxes.removeAt(i);
            return;
        }
    }
}
