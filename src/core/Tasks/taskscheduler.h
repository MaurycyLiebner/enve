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

#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H
#include <QObject>
#include "updatable.h"
#include "gpupostprocessor.h"
#include "taskquehandler.h"
class Canvas;
class CpuExecController;
class HddExecController;
class ExecController;

class TaskScheduler : public QObject {
    Q_OBJECT
public:
    TaskScheduler();
    ~TaskScheduler();
    static TaskScheduler* sInstance;

    static TaskScheduler * sGetInstance() { return sInstance; }
    static void sSetFreeThreadsForCpuTasksAvailableFunc(
            const std::function<void(void)>& func) {
        sInstance->setFreeThreadsForCpuTasksAvailableFunc(func);
    }

    static void sSetAllTasksFinishedFunc(
            const std::function<void(void)>& func) {
        sInstance->setAllTasksFinishedFunc(func);
    }
    static void sClearAllFinishedFuncs() {
        sSetFreeThreadsForCpuTasksAvailableFunc(nullptr);
        sSetAllTasksFinishedFunc(nullptr);
    }

    static bool sAllTasksFinished() {
        return sInstance->allQuedTasksFinished();
    }
    static bool sAllQuedCpuTasksFinished() {
        return sInstance->allQuedCpuTasksFinished();
    }

    static bool sCpuTasksBeingProcessed() {
        return sInstance->cpuTasksBeingProcessed();
    }

    static bool sHddTasksBeingProcessed() {
        return sInstance->hddTaskBeingProcessed();
    }

    static bool sAllQuedHddTasksFinished() {
        return sInstance->allQuedHddTasksFinished();
    }

    static void sClearTasks() {
        sInstance->clearTasks();
    }

    void initializeGpu();

    void queTasks();
    void queCpuTaskFastTrack(const stdsptr<eTask>& task);
    void queCpuTask(const stdsptr<eTask> &task);

    void scheduleCpuTask(const stdsptr<eTask> &task);
    void scheduleHddTask(const stdsptr<eTask> &task);
    void scheduleGpuTask(const stdsptr<eTask>& task);

    void clearTasks() {
        for(const auto& cpuTask : mScheduledCpuTasks)
            cpuTask->cancel();
        mScheduledCpuTasks.clear();

        for(const auto& hddTask : mScheduledHddTasks)
            hddTask->cancel();
        mScheduledHddTasks.clear();

        mQuedCpuTasks.clear();

        for(const auto& hddTask : mQuedHddTasks)
            hddTask->cancel();
        mQuedHddTasks.clear();

        mGpuPostProcessor.clear();

        callAllTasksFinishedFunc();
    }

    void switchToBackupHddExecutor();

    void afterHddTaskFinished(const stdsptr<eTask>& finishedTask,
                              ExecController * const controller);

    void afterCpuTaskFinished(const stdsptr<eTask>& task,
                              ExecController * const controller);

    void setFreeThreadsForCpuTasksAvailableFunc(
            const std::function<void(void)>& func) {
        mFreeThreadsForCpuTasksAvailableFunc = func;
    }

    void setAllTasksFinishedFunc(
            const std::function<void(void)>& func) {
        mAllTasksFinishedFunc = func;
    }

    bool allQuedTasksFinished() const {
        return allQuedCpuTasksFinished() &&
               allQuedHddTasksFinished() &&
               allQuedGpuTasksFinished();
    }

    bool allQuedGpuTasksFinished() const {
        return mGpuPostProcessor.allDone();
    }

    bool allQuedCpuTasksFinished() const {
        return !cpuTasksBeingProcessed() && mQuedCpuTasks.isEmpty();
    }

    bool allQuedHddTasksFinished() const {
        return !hddTaskBeingProcessed() && mQuedHddTasks.isEmpty();
    }

    bool cpuTasksBeingProcessed() const {
        return busyCpuThreads() > 0;
    }

    bool hddTaskBeingProcessed() const {
        return busyHddThreads() > 0;
    }

    int busyHddThreads() const {
        const int totalThreads = mHddExecs.count();
        const int freeBackup = mFreeBackupHddExecs.count();
        const int freeMain = mHddThreadBusy ? 0 : 1;
        return totalThreads - freeBackup - freeMain;
    }

    int busyCpuThreads() const {
        return mCpuExecutors.count() - mFreeCpuExecs.count();
    }

    void afterCpuGpuTaskFinished();

    void setAlwaysQue(const bool alwaysQue) {
        mAlwaysQue = alwaysQue;
    }
signals:
    void finishedAllQuedTasks() const;
    void hddUsageChanged(bool);
    void gpuUsageChanged(bool);
    void cpuUsageChanged(int);
private:
    void queScheduledCpuTasks();
    void queScheduledHddTasks();

    void processNextQuedHddTask();
    void processNextQuedCpuTask();
    bool processNextQuedGpuTask();
    void processNextTasks();

    void tryProcessingNextQuedHddTask();

    bool shouldQueMoreCpuTasks() const;
    bool shouldQueMoreHddTasks() const;
    bool overflowed() const;

    void callFreeThreadsForCpuTasksAvailableFunc() const {
        if(mFreeThreadsForCpuTasksAvailableFunc) {
            mFreeThreadsForCpuTasksAvailableFunc();
        }
    }

    void callAllTasksFinishedFunc() const {
        if(allQuedTasksFinished()) {
            if(mAllTasksFinishedFunc) {
                mAllTasksFinishedFunc();
            }
            emit finishedAllQuedTasks();
        }
    }

    bool mHddThreadBusy = false;

    bool mAlwaysQue = false;
    bool mCpuQueing = false;
    TaskQueHandler mQuedCpuTasks;

    QList<stdsptr<eTask>> mScheduledCpuTasks;
    QList<stdsptr<eTask>> mScheduledHddTasks;
    QList<stdsptr<eTask>> mQuedHddTasks;

    HddExecController *createNewBackupHddExecutor();

    HddExecController *mHddExecutor = nullptr;
    QList<HddExecController*> mFreeBackupHddExecs;
    QList<HddExecController*> mHddExecs;

    QList<CpuExecController*> mFreeCpuExecs;
    QList<CpuExecController*> mCpuExecutors;

    std::function<void(void)> mFreeThreadsForCpuTasksAvailableFunc;
    std::function<void(void)> mAllTasksFinishedFunc;

    GpuPostProcessor mGpuPostProcessor;
};

#endif // TASKSCHEDULER_H
