// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
