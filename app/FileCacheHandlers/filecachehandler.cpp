#include "filecachehandler.h"
#include "Boxes/boundingbox.h"
#include "filesourcescache.h"

FileDataCacheHandler::FileDataCacheHandler() {}

void FileDataCacheHandler::setFilePath(const QString &path) {
    clearCache();
    mFilePath = path;
    const QFile file(mFilePath);
    mFileMissing = !file.exists();
    afterPathChanged();
    emit pathChanged(mFilePath, mFileMissing);
    for(const auto &boxPtr : mDependentBoxes) {
        if(boxPtr) boxPtr->reloadCacheHandler();
    }
}

void FileDataCacheHandler::addDependentBox(
        BoundingBox * const dependent) {
    mDependentBoxes << dependent;
}

void FileDataCacheHandler::removeDependentBox(
        BoundingBox * const dependent) {
    for(int i = 0; i < mDependentBoxes.count(); i++) {
        const auto &boxPtr = mDependentBoxes.at(i);
        if(boxPtr == dependent) {
            mDependentBoxes.removeAt(i);
            return;
        }
    }
}

FileCacheHandler::FileCacheHandler(const QString &name) :
    mName(name) {}

FileCacheHandler::~FileCacheHandler() {
    for(const auto& data : mData) {
        if(data->decUseCount() == 0)
            FileSourcesCache::removeHandler(data);
    }
}

void FileCacheHandler::addDataHandler(
        const qsptr<FileDataCacheHandler> &data) {
    mData << data;
    data->incUseCount();
}
