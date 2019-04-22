#ifndef HDDCACHABLECONTAINER_H
#define HDDCACHABLECONTAINER_H
#include "minimalcachecontainer.h"
#include "tmpfilehandlers.h"
class Task;

class HDDCachable : public CacheContainer {
protected:
    HDDCachable() {}
    virtual int clearMemory() = 0;
    virtual stdsptr<HDDTask> createTmpFileDataSaver() = 0;
    virtual stdsptr<HDDTask> createTmpFileDataLoader() = 0;
public:
    ~HDDCachable() {
        if(mTmpFile) scheduleDeleteTmpFile();
    }

    int free_RAM_k() final {
        const int bytes = clearMemory();
        if(mTmpFile) setDataInMemory(false);
        else noDataLeft_k();
        return bytes;
    }

    Task* scheduleDeleteTmpFile() {
        if(!mTmpFile) return nullptr;
        const auto updatable =
                SPtrCreate(TmpFileDataDeleter)(mTmpFile);
        mTmpFile.reset();
        updatable->scheduleTask();
        return updatable.get();
    }

    Task* saveToTmpFile() {
        if(mSavingUpdatable || mTmpFile) return nullptr;
        mSavingUpdatable = createTmpFileDataSaver();
        mSavingUpdatable->scheduleTask();
        return mSavingUpdatable.get();
    }

    Task* scheduleLoadFromTmpFile() {
        if(storesDataInMemory() || !mTmpFile) return nullptr;
        if(mLoadingUpdatable) return mLoadingUpdatable.get();

        mLoadingUpdatable = createTmpFileDataLoader();
        mLoadingUpdatable->scheduleTask();
        return mLoadingUpdatable.get();
    }

    void setDataSavedToTmpFile(const qsptr<QTemporaryFile> &tmpFile) {
        mSavingUpdatable.reset();
        mTmpFile = tmpFile;
    }

    bool storesDataInMemory() const {
        return mDataInMemory;
    }
protected:
    void afterDataLoadedFromTmpFile() {
        setDataInMemory(true);
        mLoadingUpdatable.reset();
        if(!blocked()) addToMemoryManagment();
    }

    void afterDataReplaced() {
        setDataInMemory(true);
        updateInMemoryManagment();
        if(mTmpFile) scheduleDeleteTmpFile();
    }

    void setDataInMemory(const bool& dataInMemory) {
        mDataInMemory = dataInMemory;
    }

    qsptr<QTemporaryFile> mTmpFile;
private:
    bool mDataInMemory = false;
    stdsptr<Task> mLoadingUpdatable;
    stdsptr<Task> mSavingUpdatable;
};

class HDDCachablePersistent : public HDDCachable {
protected:
    HDDCachablePersistent() {}
public:
    void noDataLeft_k() final {}
};

#endif // HDDCACHABLECONTAINER_H
