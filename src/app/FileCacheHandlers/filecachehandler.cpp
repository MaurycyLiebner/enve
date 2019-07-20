#include "filecachehandler.h"
#include "Boxes/boundingbox.h"
#include "filesourcescache.h"
#include "filedatacachehandler.h"
#include "GUI/filesourcelist.h"

QList<qsptr<FileCacheHandler>> FileCacheHandler::sFileHandlers;

FileCacheHandler::FileCacheHandler() {}

bool FileCacheHandler::sRemoveFileHandler(const qsptr<FileCacheHandler> &fh) {
    FileSourceListVisibleWidget::sRemoveFromWidgets(fh.get());
    return sFileHandlers.removeOne(fh);
}

void FileCacheHandler::sAddToWidgets(FileCacheHandler * const fh) {
    FileSourceListVisibleWidget::sAddToWidgets(fh);
}
