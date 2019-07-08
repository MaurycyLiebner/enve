#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include <unordered_map>
#include <QString>
#include <QList>
#include "FileCacheHandlers/filecachehandler.h"

class FileSourceListVisibleWidget;

bool hasVideoExt(const QString &filename);
bool hasSoundExt(const QString &filename);
bool hasVectorExt(const QString &filename);
bool hasImageExt(const QString &filename);
bool hasAvExt(const QString &filename);

bool isVideoExt(const QString &extension);
bool isSoundExt(const QString &extension);
bool isVectorExt(const QString &extension);
bool isImageExt(const QString &extension);
bool isEvExt(const QString &extension);


namespace FileSourcesCache {
    extern QList<FileSourceListVisibleWidget*> sFileSourceListVisibleWidgets;
    extern QList<qsptr<FileDataCacheHandler>> sFileCacheHandlers;

    void addFileSourceListVisibleWidget(
            FileSourceListVisibleWidget *wid);
    void removeFileSourceListVisibleWidget(
            FileSourceListVisibleWidget *wid);
    template<typename T>
    T *getHandlerForFilePath(const QString &filePath);
    void clearAll();
    int getFileCacheListCount();

    void removeHandler(const qsptr<FileDataCacheHandler> &handler);
    void removeHandlerFromListWidgets(
            FileDataCacheHandler *handlerPtr);

    void addNewHandler(const qsptr<FileDataCacheHandler>& handler);
};

template<typename T>
T *FileSourcesCache::getHandlerForFilePath(const QString &filePath) {
    for(const auto &handler : sFileCacheHandlers) {
        if(handler->getFilePath() == filePath) {
            const auto handlerT = dynamic_cast<T*>(handler.get());
            if(handlerT) return handlerT;
        }
    }
    const auto handler = SPtrCreateTemplated(T)();
    addNewHandler(handler);
    try {
        handler->setFilePath(filePath);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
        FileSourcesCache::removeHandler(handler);
        return nullptr;
    }
    return handler.get();
}

#endif // FILESOURCESCACHE_H
