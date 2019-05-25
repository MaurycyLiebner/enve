#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include <unordered_map>
#include <QString>
#include <QList>
#include "Boxes/rendercachehandler.h"
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
    void addFileSourceListVisibleWidget(
            FileSourceListVisibleWidget *wid);
    void removeFileSourceListVisibleWidget(
            FileSourceListVisibleWidget *wid);
    template<typename T>
    T *getHandlerForFilePath(const QString &filePath);
    void clearAll();
    int getFileCacheListCount();

    void removeHandler(const stdsptr<FileCacheHandler> &handler);
    void addHandlerToHandlersList(
            const stdsptr<FileCacheHandler>& handlerPtr);
    void addHandlerToListWidgets(
            FileCacheHandler *handlerPtr);
    void removeHandlerFromListWidgets(
            FileCacheHandler *handlerPtr);

    template<typename T, typename... Args>
    T* createNewHandler(Args && ...arguments) {
        auto newHandler = SPtrCreateTemplated(T)(arguments...);
        FileSourcesCache::addHandlerToHandlersList(newHandler);
        FileSourcesCache::addHandlerToListWidgets(newHandler.get());
        return newHandler.get();
    }

    namespace {
        QList<FileSourceListVisibleWidget*> mFileSourceListVisibleWidgets;
        QList<stdsptr<FileCacheHandler>> mFileCacheHandlers;
    }
};

template<typename T>
T *FileSourcesCache::getHandlerForFilePath(const QString &filePath) {
    for(const auto &handler : mFileCacheHandlers) {
        if(handler->getFilePath() == filePath) {
            const auto handlerT = dynamic_cast<T*>(handler.get());
            if(handlerT) return handlerT;
        }
    }
    const auto handler = FileSourcesCache::createNewHandler<T>();
    try {
        handler->setFilePath(filePath);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
        const auto handlerSPtr = GetAsSPtr(handler, FileCacheHandler);
        FileSourcesCache::removeHandler(handlerSPtr);
        return nullptr;
    }
    return handler;
}

#endif // FILESOURCESCACHE_H
