#ifndef FILEDATACACHEHANDLER_H
#define FILEDATACACHEHANDLER_H
#include "smartPointers/selfref.h"

class FileDataCacheHandler : public SelfRef {
    Q_OBJECT
protected:
    FileDataCacheHandler();
    virtual void afterSourceChanged() = 0;
public:
    ~FileDataCacheHandler() {
        sDataHandlers.removeOne(this);
    }
    virtual void clearCache() = 0;
    virtual void replace() = 0;

    void reload() {
        clearCache();
        afterSourceChanged();
        emit sourceChanged();
    }

    bool setFilePath(const QString &path);

    const QString &getFilePath() const {
        return mFilePath;
    }

    bool isFileMissing() { return mFileMissing; }

    template<typename T>
    static T *sGetDataHandler(const QString &filePath);
signals:
    void pathChanged(const QString& path, bool missing);
    void sourceChanged();
protected:
    bool mFileMissing = false;
    int mUseCount = 0;
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
    const auto handler = SPtrCreateTemplated(T)();
    sAddDataHandler(handler);
    try {
        handler->setFilePath(filePath);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
        sRemoveDataHandler(handler);
        return nullptr;
    }
    return handler.get();
}
#endif // FILEDATACACHEHANDLER_H
