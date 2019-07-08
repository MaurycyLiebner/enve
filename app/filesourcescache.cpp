#include "filesourcescache.h"
#include "FileCacheHandlers/imagecachehandler.h"
#include "FileCacheHandlers/videocachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "GUI/mainwindow.h"
#include "Boxes/boundingbox.h"
#include "GUI/filesourcelist.h"
#include "Boxes/videobox.h"
#include <QFileDialog>

QList<FileSourceListVisibleWidget*> FileSourcesCache::sFileSourceListVisibleWidgets;
QList<qsptr<FileDataCacheHandler>> FileSourcesCache::sFileCacheHandlers;

void FileSourcesCache::addFileSourceListVisibleWidget(
        FileSourceListVisibleWidget *wid) {
    sFileSourceListVisibleWidgets << wid;
}

void FileSourcesCache::removeFileSourceListVisibleWidget(
        FileSourceListVisibleWidget *wid) {
    sFileSourceListVisibleWidgets.removeOne(wid);
}

void FileSourcesCache::addNewHandler(const qsptr<FileDataCacheHandler> &handler) {
    sFileCacheHandlers.append(handler);
    for(const auto& wid : sFileSourceListVisibleWidgets) {
        wid->addCacheHandlerToList(handler.get());
    }
}

void FileSourcesCache::removeHandler(const qsptr<FileDataCacheHandler>& handler) {
    sFileCacheHandlers.removeOne(handler);
    for(const auto& wid : sFileSourceListVisibleWidgets) {
        wid->removeCacheHandlerFromList(handler.get());
    }
}

void FileSourcesCache::removeHandlerFromListWidgets(FileDataCacheHandler *handlerPtr) {
    for(const auto& wid : sFileSourceListVisibleWidgets) {
        wid->removeCacheHandlerFromList(handlerPtr);
    }
}

void FileSourcesCache::clearAll() {
    for(const auto &wid : sFileSourceListVisibleWidgets) {
        wid->clear();
    }
    for(const auto &handler : sFileCacheHandlers) {
        handler->clearCache();
    }
    sFileCacheHandlers.clear();
}

int FileSourcesCache::getFileCacheListCount() {
    return sFileCacheHandlers.count();
}

bool isVideoExt(const QString &extension) {
    return extension == "avi" ||
           extension == "mp4" ||
           extension == "mov" ||
           extension == "mkv" ||
           extension == "m4v";
}

bool isSoundExt(const QString &extension) {
    return extension == "mp3" ||
           extension == "wav";
}

bool isVectorExt(const QString &extension) {
    return extension == "svg";
}

bool isImageExt(const QString &extension) {
    return extension == "png" ||
           extension == "jpg";
}

bool isEvExt(const QString &extension) {
    return extension == "ev";
}

bool hasVideoExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isVideoExt(extension);
}

bool hasSoundExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isSoundExt(extension);
}

bool hasVectorExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isVectorExt(extension);
}

bool hasImageExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isImageExt(extension);
}

bool hasAvExt(const QString &filename) {
    QString extension = filename.split(".").last();
    return isEvExt(extension);
}
