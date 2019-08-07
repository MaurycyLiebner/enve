#ifndef FILEDATACACHEHANDLER_H
#define FILEDATACACHEHANDLER_H
#include "smartPointers/selfref.h"
#include "smartPointers/ememory.h"

class FileDataCacheHandler : public SelfRef {
    Q_OBJECT
protected:
    FileDataCacheHandler();
    virtual void afterSourceChanged() = 0;
public:
    ~FileDataCacheHandler();
    virtual void clearCache() = 0;

    void reload() {
        clearCache();
        mFileMissing = !QFile(mFilePath).exists();
        afterSourceChanged();
        emit sourceChanged();
    }

    void setFilePath(const QString &path);

    const QString &getFilePath() const {
        return mFilePath;
    }

    bool isFileMissing() { return mFileMissing; }

    template<typename T>
    static T *sGetDataHandler(const QString &filePath);
    template<typename T>
    static qsptr<T> sGetCreateDataHandler(const QString &filePath);
    template<typename T>
    static qsptr<T> sCreateDataHandler(const QString &filePath);
signals:
    void sourceChanged();
protected:
    bool mFileMissing = false;
    QString mFilePath;
private:
    static QList<FileDataCacheHandler*> sDataHandlers;
};

template<typename T>
T *FileDataCacheHandler::sGetDataHandler(const QString &filePath) {
    for(const auto &handler : sDataHandlers) {
        if(handler->getFilePath() == filePath) {
            const auto handlerT = dynamic_cast<T*>(handler);
            if(handlerT) return handlerT;
        }
    }
    return nullptr;
}

template<typename T>
qsptr<T> FileDataCacheHandler::sCreateDataHandler(const QString &filePath) {
    const auto handler = enve::make_shared<T>();
    try {
        handler->setFilePath(filePath);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
        return nullptr;
    }
    return handler;
}

template<typename T>
qsptr<T> FileDataCacheHandler::sGetCreateDataHandler(const QString &filePath) {
    const auto get = sGetDataHandler<T>(filePath);
    if(get) return get->template ref<T>();
    return sCreateDataHandler<T>(filePath);
}

#endif // FILEDATACACHEHANDLER_H
