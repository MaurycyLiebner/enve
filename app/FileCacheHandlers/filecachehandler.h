#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "filedatacachehandler.h"
#include "smartPointers/sharedpointerdefs.h"

class FileCacheHandler : public SelfRef {
    Q_OBJECT
protected:
    FileCacheHandler();

    virtual void afterPathSet(const QString& path) = 0;
    virtual void reload() = 0;
public:
    virtual void replace() = 0;

    void reloadAction() {
        reload();
        emit reloaded();
    }

    const QString& path() const { return mPath; }
    bool fileMissing() const { return mFileMissing; }

    template <typename T>
    static T *sGetFileHandler(const QString &filePath);
    static bool sRemoveFileHandler(const qsptr<FileCacheHandler> &fh);
    static void sClear() { sFileHandlers.clear(); }
signals:
    void pathChanged(const QString& newPath);
    void reloaded();
protected:
    void setPath(const QString& path) {
        if(mPath == path) return;
        mPath = path;
        afterPathSet(path);
        emit pathChanged(path);
    }
private:
    static void sAddToWidgets(FileCacheHandler* const fh);

    static QList<qsptr<FileCacheHandler>> sFileHandlers;
    QString mPath; // Usually filename
    bool mFileMissing = false;
};

template <typename T>
T *FileCacheHandler::sGetFileHandler(const QString &filePath) {
    for(const auto& fh : sFileHandlers) {
        if(filePath == fh->path()) return static_cast<T*>(fh.get());
    }
    const auto fh = SPtrCreateTemplated(T)();
    try {
        fh->setPath(filePath);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
        return nullptr;
    }
    sFileHandlers.append(fh);
    sAddToWidgets(fh.get());

    return fh.get();
}



#endif // FILECACHEHANDLER_H
