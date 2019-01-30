#include "filecachehandler.h"
#include "Boxes/boundingbox.h"
#include "filesourcescache.h"

FileCacheHandler::FileCacheHandler(const QString &filePath,
                                   const bool &visibleInListWidgets) {
    mVisibleInListWidgets = visibleInListWidgets;
    mFilePath = filePath;
    QFile file(mFilePath);
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
    QFile file(mFilePath);
    mFileMissing = !file.exists();
    foreach(const auto &boxPtr, mDependentBoxes) {
        BoundingBox *box = boxPtr.data();
        if(!box) continue;
        box->reloadCacheHandler();
    }
}

void FileCacheHandler::addDependentBox(BoundingBox *dependent) {
    mDependentBoxes << dependent;
}

void FileCacheHandler::removeDependentBox(BoundingBox *dependent) {
    for(int i = 0; i < mDependentBoxes.count(); i++) {
        const auto &boxPtr = mDependentBoxes.at(i);
        if(boxPtr == dependent) {
            mDependentBoxes.removeAt(i);
            return;
        }
    }
}
