#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "updatable.h"
class BoundingBox;

class FileCacheHandler : public StdSelfRef {
protected:
    FileCacheHandler();
public:
    virtual void clearCache();
    virtual void replace() {}

    virtual void setFilePath(const QString &path) {
        mFilePath = path;
        const QFile file(mFilePath);
        mFileMissing = !file.exists();
    }

    const QString &getFilePath() {
        return mFilePath;
    }

    void planScheduleUpdateForAllDependent();
    void addDependentBox(BoundingBox * const dependent);
    void removeDependentBox(BoundingBox * const dependent);

    bool isFileMissing() {
        return mFileMissing;
    }
protected:
    bool mFileMissing = false;
    QList<qptr<BoundingBox>> mDependentBoxes;
    QString mFilePath;
};

#endif // FILECACHEHANDLER_H
