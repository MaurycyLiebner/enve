#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "updatable.h"
class BoundingBox;

class FileDataCacheHandler : public SelfRef {
    Q_OBJECT
protected:
    FileDataCacheHandler();
    virtual void afterPathChanged() = 0;
public:
    virtual void clearCache() = 0;
    virtual void replace() = 0;

    void setFilePath(const QString &path);

    const QString &getFilePath() const {
        return mFilePath;
    }

    void planScheduleUpdateForAllDependent();
    void addDependentBox(BoundingBox * const dependent);
    void removeDependentBox(BoundingBox * const dependent);

    bool isFileMissing() { return mFileMissing; }

    //! @brief How many FileCacheHandlers reference this
    int useCount() const { return mUseCount; }
    int incUseCount() { return ++mUseCount; }
    int decUseCount() { return --mUseCount; }
signals:
    void pathChanged(const QString& path, bool missing);
protected:
    bool mFileMissing = false;
    int mUseCount = 0;
    QList<qptr<BoundingBox>> mDependentBoxes;
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
