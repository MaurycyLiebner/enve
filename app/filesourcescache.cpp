#include "filesourcescache.h"
#include "FileCacheHandlers/imagecachehandler.h"
#include "FileCacheHandlers/videocachehandler.h"
#include "Boxes/rendercachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "GUI/mainwindow.h"
#include "Boxes/boundingbox.h"
#include "GUI/filesourcelist.h"
#include "Boxes/videobox.h"
#include <QFileDialog>

void FileSourcesCache::addFileSourceListVisibleWidget(
        FileSourceListVisibleWidget *wid) {
    mFileSourceListVisibleWidgets << wid;
}

void FileSourcesCache::removeFileSourceListVisibleWidget(
        FileSourceListVisibleWidget *wid) {
    mFileSourceListVisibleWidgets.removeOne(wid);
}

void FileSourcesCache::addHandlerToHandlersList(
        const stdsptr<FileCacheHandler> &handlerPtr) {
    mFileCacheHandlers.append(handlerPtr);
}

void FileSourcesCache::addHandlerToListWidgets(FileCacheHandler *handlerPtr) {
    for(FileSourceListVisibleWidget *wid : mFileSourceListVisibleWidgets) {
        wid->addCacheHandlerToList(handlerPtr);
    }
}

FileCacheHandler *FileSourcesCache::getHandlerForFilePath(
        const QString &filePath) {
    for(const auto &handler : mFileCacheHandlers) {
        if(handler->getFilePath() == filePath) {
            return handler.get();
        }
    }
    QString ext = filePath.split(".").last();
    if(isVideoExt(ext)) {
        return FileSourcesCache::
                createNewHandler<VideoCacheHandler>(filePath);
    } else if(isImageExt(ext)) {
        return FileSourcesCache::
                createNewHandler<ImageCacheHandler>(filePath);
    }
    return nullptr;
}

void FileSourcesCache::removeHandler(const stdsptr<FileCacheHandler>& handler) {
    mFileCacheHandlers.removeOne(handler);
    for(FileSourceListVisibleWidget *wid : mFileSourceListVisibleWidgets) {
        wid->removeCacheHandlerFromList(handler.get());
    }
}

void FileSourcesCache::removeHandlerFromListWidgets(FileCacheHandler *handlerPtr) {
    for(FileSourceListVisibleWidget *wid : mFileSourceListVisibleWidgets) {
        wid->removeCacheHandlerFromList(handlerPtr);
    }
}

void FileSourcesCache::clearAll() {
    for(const auto &handler : mFileCacheHandlers) {
        handler->clearCache();
    }
    mFileCacheHandlers.clear();
}

int FileSourcesCache::getFileCacheListCount() {
    return mFileCacheHandlers.count();
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

bool isAvExt(const QString &extension) {
    return extension == "av";
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
    return isAvExt(extension);
}
