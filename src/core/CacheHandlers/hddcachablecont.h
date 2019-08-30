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
#include "cachecontainer.h"
#include "tmpdeleter.h"
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
        if(!mTmpFile && !mTmpSaveTask) noDataLeft_k();
        return bytes;
    }

    eTask* scheduleDeleteTmpFile() {
        if(!mTmpFile) return nullptr;
        const auto updatable =
                enve::make_shared<TmpDeleter>(mTmpFile);
        mTmpFile.reset();
        updatable->scheduleTask();
        return updatable.get();
    }

    eTask* scheduleSaveToTmpFile() {
        if(mTmpSaveTask || mTmpFile) return nullptr;
        mTmpSaveTask = createTmpFileDataSaver();
        mTmpSaveTask->scheduleTask();
        return mTmpSaveTask.get();
    }

    eTask* scheduleLoadFromTmpFile() {
        if(storesDataInMemory()) return nullptr;
        if(mTmpLoadTask) return mTmpLoadTask.get();
        if(!mTmpSaveTask && !mTmpFile) return nullptr;

        mTmpLoadTask = createTmpFileDataLoader();
        if(mTmpSaveTask)
            mTmpSaveTask->addDependent(mTmpLoadTask.get());
        mTmpLoadTask->scheduleTask();
        return mTmpLoadTask.get();
    }

    void setDataSavedToTmpFile(const qsptr<QTemporaryFile> &tmpFile) {
        mTmpSaveTask.reset();
        mTmpFile = tmpFile;
    }

    bool storesDataInMemory() const {
        return mDataInMemory;
    }

    qsptr<QTemporaryFile> getTmpFile() const { return mTmpFile; }
protected:
    void afterDataLoadedFromTmpFile() {
        setDataInMemory(true);
        mTmpLoadTask.reset();
        if(!inUse()) addToMemoryManagment();
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
    stdsptr<eTask> mTmpLoadTask;
    stdsptr<eTask> mTmpSaveTask;
};

#endif // HddCACHABLECONT_H
