#include "filecachehandler.h"
#include "Boxes/boundingbox.h"
#include "filesourcescache.h"

FileCacheHandler::FileCacheHandler(const QString &filePath,
                                   const bool &visibleInListWidgets) {
    mVisibleInListWidgets = visibleInListWidgets;
    mFilePath = filePath;
    const QFile file(mFilePath);
    mFileMissing = !file.exists();
}

void FileCacheHandler::setVisibleInListWidgets(const bool &bT) {
    if(bT == mVisibleInListWidgets) return;
    mVisibleInListWidgets = bT;
    if(bT) {
        FileSourcesCache::addHandlerToListWidgets(this);
    } else {
        FileSourcesCache::removeHandlerFromListWidgets(this);
    }
}

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
