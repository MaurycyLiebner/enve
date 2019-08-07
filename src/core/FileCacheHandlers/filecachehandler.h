#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "filedatacachehandler.h"
#include "smartPointers/ememory.h"

class FileCacheHandler : public SelfRef {
    friend class FilesHandler;
    Q_OBJECT
protected:
    FileCacheHandler();

    virtual void afterPathSet(const QString& path) = 0;
    virtual void reload() = 0;
public:
    virtual void replace(QWidget* const parent) = 0;

    void reloadAction() {
        reload();
        emit reloaded();
    }

    const QString& path() const { return mPath; }
    bool fileMissing() const { return mFileMissing; }
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

    bool mFileMissing = false;
    QString mPath; // filename / dirname
};


#endif // FILECACHEHANDLER_H
