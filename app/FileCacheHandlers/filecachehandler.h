#ifndef FILECACHEHANDLER_H
#define FILECACHEHANDLER_H
#include "updatable.h"
class BoundingBox;

class FileCacheHandler : public _HDDTask {
public:
    FileCacheHandler(const QString &filePath,
                     const bool &visibleInListWidgets = true);

    const QString &getFilePath() {
        return mFilePath;
    }

    virtual void clearCache();

    void scheduleUpdateForAllDependent();
    void addDependentBox(BoundingBox *dependent);
    void removeDependentBox(BoundingBox *dependent);
    virtual void replace() {}
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
