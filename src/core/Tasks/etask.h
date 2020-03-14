// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#ifndef ETASK_H
#define ETASK_H

#include "smartPointers/ememory.h"

#include "glhelpers.h"
#include "hardwareenums.h"
#include "switchablecontext.h"
#include "../ReadWrite/basicreadwrite.h"

enum class eTaskState {
    created,
    qued,
    processing,
    finished,
    canceled,
    waiting
};

class eTask : public StdSelfRef {
    friend class TaskScheduler;
    friend class Que;
    template <typename T> friend class TaskCollection;
protected:
    eTask() {}

    virtual void queTaskNow() = 0;
    virtual void afterQued() {}
    virtual void beforeProcessing(const Hardware) {}
    virtual void afterProcessing() {}
    virtual void afterCanceled() {}
    virtual void handleException() {}
public:
    ~eTask() { cancelDependent(); }

    struct Dependent {
        std::function<void()> fFinished;
        std::function<void()> fCanceled;
    };

    virtual HardwareSupport hardwareSupport() const = 0;
    virtual void processGpu(QGL33 * const gl,
                            SwitchableContext &context) = 0;
    virtual void process() = 0;

    virtual bool nextStep() { return false; }

    bool queTask();
    bool isQued() { return mState == eTaskState::qued; }

    bool isActive() { return mState != eTaskState::created &&
                             mState != eTaskState::finished; }

    void aboutToProcess(const Hardware hw);
    void finishedProcessing();
    bool readyToBeProcessed();

    void addDependent(eTask * const task);
    void addDependent(const Dependent& func);

    bool finished();
    eTaskState getState() const { return mState; }

    void decDependencies();
    void incDependencies();

    bool waitingToCancel() const { return mCancel; }
    void cancel();

    void setException(const std::exception_ptr& exception);
    bool unhandledException() const;
    std::exception_ptr takeException();
protected:
    eTaskState mState = eTaskState::created;
private:
    void tellDependentThatFinished();
    void cancelDependent();

    bool mCancel = false;
    int mNDependancies = 0;
    QList<stdptr<eTask>> mDependent;
    QList<Dependent> mDependentF;
    std::exception_ptr mUpdateException;
};

#endif // ETASK_H
