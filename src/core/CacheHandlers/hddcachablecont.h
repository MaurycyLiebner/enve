#ifndef HddCACHABLECONT_H
#define HddCACHABLECONT_H
#include "minimalcachecontainer.h"
#include "tmpfilehandlers.h"
class eTask;

class HddCachable : public CacheContainer {
protected:
    HddCachable() {}
    virtual int clearMemory() = 0;
    virtual stdsptr<eHddTask> createTmpFileDataSaver() = 0;
    virtual stdsptr<eHddTask> createTmpFileDataLoader() = 0;
public:
    ~HddCachable() {
        if(mTmpFile) scheduleDeleteTmpFile();
    }

    int free_RAM_k() final {
        const int bytes = clearMemory();
        setDataInMemory(false);
        if(!mTmpFile) noDataLeft_k();
        return bytes;
    }

    eTask* scheduleDeleteTmpFile() {
        if(!mTmpFile) return nullptr;
        const auto updatable =
                enve::make_shared<TmpFileDataDeleter>(mTmpFile);
        mTmpFile.reset();
        updatable->scheduleTask();
        return updatable.get();
    }

    eTask* saveToTmpFile() {
        if(mSavingUpdatable || mTmpFile) return nullptr;
        mSavingUpdatable = createTmpFileDataSaver();
        mSavingUpdatable->scheduleTask();
        return mSavingUpdatable.get();
    }

    eTask* scheduleLoadFromTmpFile() {
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

    void setDataInMemory(const bool dataInMemory) {
        mDataInMemory = dataInMemory;
    }

    qsptr<QTemporaryFile> mTmpFile;
private:
    bool mDataInMemory = false;
    stdsptr<eTask> mLoadingUpdatable;
    stdsptr<eTask> mSavingUpdatable;
};

class HddCachablePersistent : public HddCachable {
protected:
    HddCachablePersistent() {}
public:
    void noDataLeft_k() final {}
};

#endif // HddCACHABLECONT_H
