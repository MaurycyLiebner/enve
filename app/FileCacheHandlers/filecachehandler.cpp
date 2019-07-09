#include "filecachehandler.h"
#include "Boxes/boundingbox.h"
#include "filesourcescache.h"
#include "filedatacachehandler.h"
QList<FileCacheHandler*> FileCacheHandler::sFileHandlers;

FileCacheHandler::FileCacheHandler(const QString &name) :
    mName(name) {
    sFileHandlers.append(this);
}

FileCacheHandler::~FileCacheHandler() {
    sFileHandlers.removeOne(this);
}

FileCacheHandler *FileCacheHandler::sGetFileHandler(const QString &filePath) {
    for(const auto fh : sFileHandlers) {
        if(filePath == fh->name()) return fh;
    }
    return nullptr;
}
