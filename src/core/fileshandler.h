#ifndef FILESHANDLER_H
#define FILESHANDLER_H
#include <QObject>
#include "smartPointers/sharedpointerdefs.h"
#include "FileCacheHandlers/filecachehandler.h"

class FilesHandler : public QObject {
    Q_OBJECT
public:
    FilesHandler();

    void clear() {
        const auto fhs = mFileHandlers;
        for(const auto& fh : fhs) removeFileHandler(fh);
    }

    template <typename T>
    T *getFileHandler(const QString &filePath);
    bool removeFileHandler(const qsptr<FileCacheHandler> &fh);

    static FilesHandler* sInstance;
private:
    QList<qsptr<FileCacheHandler>> mFileHandlers;
signals:
    void addedCacheHandler(FileCacheHandler*);
    void removedCacheHandler(FileCacheHandler*);
};

template <typename T>
T *FilesHandler::getFileHandler(const QString &filePath) {
    for(const auto& fh : mFileHandlers) {
        if(filePath == fh->path()) return static_cast<T*>(fh.get());
    }
    const auto fh = SPtrCreateTemplated(T)();
    try {
        fh->setPath(filePath);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
        return nullptr;
    }
    mFileHandlers.append(fh);
    emit addedCacheHandler(fh.get());

    return fh.get();
}


#endif // FILESHANDLER_H
