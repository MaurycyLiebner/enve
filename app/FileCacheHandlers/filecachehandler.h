#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "updatable.h"
class BoundingBox;

class FileDataCacheHandler : public SelfRef {
    Q_OBJECT
protected:
    FileDataCacheHandler();
    virtual void afterSourceChanged() = 0;
public:
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

    //! @brief How many FileCacheHandlers reference this
    int useCount() const { return mUseCount; }
    int incUseCount() { return ++mUseCount; }
    int decUseCount() { return --mUseCount; }
signals:
    void pathChanged(const QString& path, bool missing);
    void sourceChanged();
protected:
    bool mFileMissing = false;
    int mUseCount = 0;
    QString mFilePath;
};

class FileCacheHandler : public StdSelfRef {
protected:
    FileCacheHandler(const QString& name);
    ~FileCacheHandler();

    void addDataHandler(const qsptr<FileDataCacheHandler>& data);
public:
    const QString& name() const { return mName; }
private:
    QList<qsptr<FileDataCacheHandler>> mData;
    const QString mName; // Usually filename
};

#endif // FILECACHEHANDLER_H
