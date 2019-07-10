#include "filedatacachehandler.h"
#include <QFile>
QList<FileDataCacheHandler*> FileDataCacheHandler::sDataHandlers;

FileDataCacheHandler::FileDataCacheHandler() {
    sDataHandlers.append(this);
}

FileDataCacheHandler::~FileDataCacheHandler() {
    sDataHandlers.removeOne(this);
}

bool FileDataCacheHandler::setFilePath(const QString &path) {
    if(mFilePath == path) return false;
    mFilePath = path;
    const QFile file(mFilePath);
    mFileMissing = !file.exists();
    reload();
    emit pathChanged(mFilePath, mFileMissing);
    return true;
}
