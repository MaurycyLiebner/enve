#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "updatable.h"
class BoundingBox;

class FileCacheHandler : public StdSelfRef {
protected:
    FileCacheHandler(const QString &filePath,
                     const bool &visibleInListWidgets = true);
public:
    virtual void clearCache();
    virtual void replace() {}

    const QString &getFilePath() {
        return mFilePath;
    }

    void scheduleUpdateForAllDependent();
    void addDependentBox(BoundingBox * const dependent);
    void removeDependentBox(BoundingBox * const dependent);
    void setVisibleInListWidgets(const bool &bT);

    bool isFileMissing() {
        return mFileMissing;
    }
protected:
    bool mFileMissing = false;
    bool mVisibleInListWidgets;
    QList<qptr<BoundingBox>> mDependentBoxes;
    QString mFilePath;
    QString mUpdateFilePath;
};

#endif // FILECACHEHANDLER_H
