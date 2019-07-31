#include "fileshandler.h"

FilesHandler* FilesHandler::sInstance = nullptr;

FilesHandler::FilesHandler() {
    Q_ASSERT(!sInstance);
    sInstance = this;
}

bool FilesHandler::removeFileHandler(const qsptr<FileCacheHandler> &fh) {
    const auto result = mFileHandlers.removeOne(fh);
    if(result) emit removedCacheHandler(fh.get());
    return result;
}
