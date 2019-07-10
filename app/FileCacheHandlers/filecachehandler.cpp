#include "filecachehandler.h"
#include "Boxes/boundingbox.h"
#include "filesourcescache.h"
#include "filedatacachehandler.h"
QList<qsptr<FileCacheHandler>> FileCacheHandler::sFileHandlers;

FileCacheHandler::FileCacheHandler() {}

bool FileCacheHandler::sRemoveFileHandler(const qsptr<FileCacheHandler> &fh) {
    return sFileHandlers.removeOne(fh);
}
