#include "filedatacachehandler.h"
#include <QFile>
QList<FileDataCacheHandler*> FileDataCacheHandler::sDataHandlers;

FileDataCacheHandler::FileDataCacheHandler() {
    sDataHandlers.append(this);
}

FileDataCacheHandler::~FileDataCacheHandler() {
    sDataHandlers.removeOne(this);
}

void FileDataCacheHandler::setFilePath(const QString &path) {
    mFilePath = path;
    const QFile file(mFilePath);
    mFileMissing = !file.exists();
    reload();
}
