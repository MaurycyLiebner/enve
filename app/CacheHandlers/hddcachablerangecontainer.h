#ifndef HDDCACHABLERANGECONTAINER_H
#define HDDCACHABLERANGECONTAINER_H
#include "rangecachecontainer.h"
#include "minimalcachehandler.h"
#include "tmpfilehandlers.h"
class _ScheduledTask;

template <typename T>
class HDDCachableRangeContainer : public RangeCacheContainer {
    friend class SoundContainerTmpFileDataSaver;
protected:
    HDDCachableRangeContainer(const FrameRange &range,
                              RangeCacheHandler * const parent) :
        RangeCacheContainer(range), mParentCacheHandler_k(parent) {}
    virtual stdsptr<_HDDTask> createTmpFileDataSaver() = 0;
    virtual stdsptr<_HDDTask> createTmpFileDataLoader() = 0;
    virtual int clearMemory() = 0;
public:
    ~HDDCachableRangeContainer() {
        if(mTmpFile) scheduleDeleteTmpFile();
    }

    int freeAndRemove_k() {
        const int bytes = getByteCount();
        mParentCacheHandler_k->removeRenderContainer(ref<T>());
        return bytes;
    }

    int freeFromMemory_k() {
        if(mTmpFile) return clearMemory();
        else return freeAndRemove_k();
    }

    _ScheduledTask* scheduleDeleteTmpFile() {
        if(!mTmpFile) return nullptr;
        const auto updatable =
                SPtrCreate(CacheContainerTmpFileDataDeleter)(mTmpFile);
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
