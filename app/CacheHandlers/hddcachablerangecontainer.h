#ifndef HDDCACHABLERANGECONTAINER_H
#define HDDCACHABLERANGECONTAINER_H
#include "rangecachecontainer.h"
#include "minimalcachehandler.h"
#include "tmpfilehandlers.h"
class _ScheduledTask;

template <typename T>
class HDDCachableRangeContainer : public RangeCacheContainer {
protected:
    HDDCachableRangeContainer() :
        mParentCacheHandler_k(nullptr) {}
    HDDCachableRangeContainer(const FrameRange &range,
                              RangeCacheHandler * const parent) :
        RangeCacheContainer(range), mParentCacheHandler_k(parent) {}
    virtual stdsptr<_HDDTask> createTmpFileDataSaver() = 0;
    virtual stdsptr<_HDDTask> createTmpFileDataLoader() = 0;
    virtual void clearDataAfterSaved() = 0;
public:
    ~HDDCachableRangeContainer() {
        if(mTmpFile) scheduleDeleteTmpFile();
    }

    _ScheduledTask* scheduleLoadFromTmpFile() {
        if(storesDataInMemory() || !mTmpFile) return nullptr;
        if(mLoadingUpdatable) return mLoadingUpdatable.get();

        mLoadingUpdatable = createTmpFileDataLoader();
        mLoadingUpdatable->scheduleTask();
        return mLoadingUpdatable.get();
    }

    bool freeAndRemove_k() {
        if(blocked()) return false;
        mParentCacheHandler_k->removeRenderContainer(ref<T>());
        return true;
    }

    bool freeFromMemory_k() {
        if(blocked()) return false;
        if(mTmpFile) {
            if(!blocked()) removeFromMemoryManagment();
            clearDataAfterSaved();
        } else return freeAndRemove_k();
        return true;
    }

    void scheduleDeleteTmpFile() {
        if(!mTmpFile) return;
        stdsptr<_ScheduledTask> updatable =
                SPtrCreate(CacheContainerTmpFileDataDeleter)(mTmpFile);
        mTmpFile.reset();
        updatable->scheduleTask();
    }

    void saveToTmpFile() {
        if(mSavingUpdatable || mTmpFile) return;
        mSavingUpdatable = createTmpFileDataSaver();
        mSavingUpdatable->scheduleTask();
    }

    void setDataSavedToTmpFile(const qsptr<QTemporaryFile> &tmpFile) {
        mSavingUpdatable.reset();
        mTmpFile = tmpFile;
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
    stdsptr<_ScheduledTask> mLoadingUpdatable;
    stdsptr<_ScheduledTask> mSavingUpdatable;

    RangeCacheHandler * const mParentCacheHandler_k;
};

#endif // HDDCACHABLERANGECONTAINER_H
