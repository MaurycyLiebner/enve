#ifndef HDDCACHABLERANGECONTAINER_H
#define HDDCACHABLERANGECONTAINER_H
#include "rangecachecontainer.h"
#include "minimalcachehandler.h"
#include "tmpfilehandlers.h"
class _ScheduledTask;

template <typename T>
class HDDCachableRangeContainer : public RangeCacheContainer {
protected:
    typedef MinimalCacheHandler<T> Handler;
    HDDCachableRangeContainer() :
        mParentCacheHandler_k(nullptr) {}
    HDDCachableRangeContainer(const FrameRange &range, Handler * const parent) :
        RangeCacheContainer(range), mParentCacheHandler_k(parent) {}
    virtual stdsptr<_HDDTask> createTmpFileDataSaver() = 0;
    virtual stdsptr<_HDDTask> createTmpFileDataLoader() = 0;
    virtual void clearDataAfterSaved() = 0;
public:
    ~HDDCachableRangeContainer() {
        if(mTmpFile) scheduleDeleteTmpFile();
    }

    _ScheduledTask* scheduleLoadFromTmpFile() {
        if(!mNoDataInMemory || !mTmpFile) return nullptr;
        if(mLoadingUpdatable) return mLoadingUpdatable.get();

        mLoadingUpdatable = createTmpFileDataLoader();
        mLoadingUpdatable->scheduleTask();
        return mLoadingUpdatable.get();
    }

    void setBlocked(const bool &bT) {
        if(bT == mBlocked) return;
        mBlocked = bT;
        if(bT) {
            if(mNoDataInMemory) {
                scheduleLoadFromTmpFile();
            } else {
                MemoryHandler::sGetInstance()->removeContainer(this);
            }
        } else {
            MemoryHandler::sGetInstance()->addContainer(this);
        }
    }

    bool freeAndRemove() {
        if(mBlocked) return false;
        mParentCacheHandler_k->removeRenderContainer(ref<T>());
        return true;
    }

    bool freeFromMemory() {
        if(mTmpFile) {
            if(!mBlocked) MemoryHandler::sGetInstance()->removeContainer(this);
            clearDataAfterSaved();
        } else return freeAndRemove();
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

    bool storesDataInMemory() const {
        return !mNoDataInMemory;
    }
protected:
    void afterDataLoadedFromTmpFile() {
        mNoDataInMemory = false;
        mLoadingUpdatable.reset();
        if(!mBlocked) MemoryHandler::sGetInstance()->addContainer(this);
    }

    void afterDataReplaced() {
        if(mNoDataInMemory) {
            mNoDataInMemory = false;
            if(!mBlocked) MemoryHandler::sGetInstance()->addContainer(this);
        }
        if(mTmpFile) scheduleDeleteTmpFile();
    }

    stdsptr<_ScheduledTask> mLoadingUpdatable;
    stdsptr<_ScheduledTask> mSavingUpdatable;

    qsptr<QTemporaryFile> mTmpFile;

    bool mNoDataInMemory = false;

    Handler * const mParentCacheHandler_k;
};

#endif // HDDCACHABLERANGECONTAINER_H
