#include "filesourcescache.h"
#include "Boxes/rendercachehandler.h"

FileSourcesCache::FileSourcesCache() {

}

void FileSourcesCache::addHandler(FileCacheHandler *handlerPtr) {
    mFileCacheHandlers.append(handlerPtr);
}

FileCacheHandler *FileSourcesCache::getHandlerForFilePath(const QString &filePath) {
    Q_FOREACH(FileCacheHandler *handler, mFileCacheHandlers) {
        if(handler->getFilePath() == filePath) {
            return handler;
        }
    }
    return NULL;
}

void FileSourcesCache::removeHandler(FileCacheHandler *handler) {
    mFileCacheHandlers.removeOne(handler);
}
