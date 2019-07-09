#include "filecachehandler.h"
#include "Boxes/boundingbox.h"
#include "filesourcescache.h"

FileDataCacheHandler::FileDataCacheHandler() {}

bool FileDataCacheHandler::setFilePath(const QString &path) {
    if(mFilePath == path) return false;
    mFilePath = path;
    const QFile file(mFilePath);
    mFileMissing = !file.exists();
    reload();
    emit pathChanged(mFilePath, mFileMissing);
    return true;
}

FileCacheHandler::FileCacheHandler(const QString &name) :
    mName(name) {}

FileCacheHandler::~FileCacheHandler() {
    for(const auto& data : mData) {
        if(data->decUseCount() == 0)
            FileSourcesCache::removeHandler(
                        GetAsSPtr(data, FileDataCacheHandler));
    }
}

void FileCacheHandler::addDataHandler(FileDataCacheHandler* const data) {
    mData << data;
    data->incUseCount();
}
