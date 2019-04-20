#ifndef HDDCACHABLECONTAINER_H
#define HDDCACHABLECONTAINER_H
#include "minimalcachecontainer.h"
#include "tmpfilehandlers.h"
class _ScheduledTask;

class HDDCachableContainer : public MinimalCacheContainer {
protected:
    HDDCachableContainer(const bool& persist) : mPersistData(persist) {}
    virtual int clearMemory() = 0;
    virtual stdsptr<_HDDTask> createTmpFileDataSaver() = 0;
    virtual stdsptr<_HDDTask> createTmpFileDataLoader() = 0;
public:
    ~HDDCachableContainer() {
        if(mTmpFile) scheduleDeleteTmpFile();
    }

    int freeFromMemory_k() {
        if(mTmpFile) {
            const int bytes = clearMemory();
            setDataInMemory(false);
            return bytes;
        } else if(mPersistData) return 0;
        else return freeAndRemove_k();
    }

    _ScheduledTask* scheduleDeleteTmpFile() {
        if(!mTmpFile) return nullptr;
        const auto updatable =
                SPtrCreate(TmpFileDataDeleter)(mTmpFile);
        mTmpFile.reset();
        updatable->scheduleTask();
        return updatable.get();
    }

    _ScheduledTask* saveToTmpFile() {
        if(mSavingUpdatable || mTmpFile) return nullptr;
        mSavingUpdatable = createTmpFileDataSaver();
        mSavingUpdatable->scheduleTask();
        return mSavingUpdatable.get();
    }

    _ScheduledTask* scheduleLoadFromTmpFile() {
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

    qsptr<QTemporaryFile> mTmpFile;
private:
    void setDataInMemory(const bool& dataInMemory) {
        mDataInMemory = dataInMemory;
    }

    const bool mPersistData;
    bool mDataInMemory = false;
    stdsptr<_ScheduledTask> mLoadingUpdatable;
    stdsptr<_ScheduledTask> mSavingUpdatable;
};

#endif // HDDCACHABLECONTAINER_H
