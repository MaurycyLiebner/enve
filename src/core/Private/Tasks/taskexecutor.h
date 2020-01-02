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

#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H
#include <QObject>
#include <QTimer>
#include <QThread>
#include "Tasks/updatable.h"

class TaskExecutor : public QObject {
    Q_OBJECT
public:
    explicit TaskExecutor() {}
    void processTask(eTask* task);
signals:
    void finishedTask(eTask*);
};

class HddTaskExecutor : public TaskExecutor {
    Q_OBJECT
public:
    explicit HddTaskExecutor() {}
signals:
    void hddPartFinished();
};

class ExecController : public QObject {
    Q_OBJECT
protected:
    ExecController(TaskExecutor * const executor,
                   QObject * const parent = nullptr) : QObject(parent),
        mExecutor(executor),
        mExecutorThread(new QThread(this)) {
        connect(this, &ExecController::processTaskSignal,
                mExecutor, &TaskExecutor::processTask);
        connect(mExecutor, &TaskExecutor::finishedTask,
                this, &ExecController::finishedTask);
        mExecutor->moveToThread(mExecutorThread);
        mExecutorThread->start();
    }
public:
    void processTask(const stdsptr<eTask>& task) {
        //if(mCurrentTask) RuntimeThrow("Previous task did not finish yet");
        //mCurrentTask = task;
        mTasks << task;//
        emit processTaskSignal(task.get());
    }

    void quit() {
        mExecutorThread->quit();
    }

    void wait() {
        mExecutorThread->wait();
//        {
//            QEventLoop loop;
//            QObject::connect(mExecutor, &TaskExecutor::finishedTask,
//                             &loop, &QEventLoop::quit);
//            loop.exec();
//        }
    }
signals:
    void processTaskSignal(eTask*);
    void finishedTaskSignal(stdsptr<eTask>, ExecController*);
protected:
    TaskExecutor * const mExecutor;
private:
    void finishedTask(eTask* const task) {
        for(int i = 0; i < mTasks.count(); i++) {//
            const auto& iTask = mTasks[i];//
            if(task == iTask.get()) {//
                emit finishedTaskSignal(iTask, this);//
                mTasks.removeAt(i);//
                break;
            }//
        }//
//        stdsptr<eTask> task;
//        task.swap(mCurrentTask);
//        emit finishedTaskSignal(task, this);
    }

    QList<stdsptr<eTask>> mTasks;
//    stdsptr<eTask> mCurrentTask;
    QThread * const mExecutorThread;
};

class CpuExecController : public ExecController {
public:
    CpuExecController(QObject * const parent = nullptr) :
        ExecController(new TaskExecutor, parent) {}
};

class HddExecController : public ExecController {
    Q_OBJECT
public:
    HddExecController(QObject * const parent = nullptr) :
        ExecController(new HddTaskExecutor, parent) {
        connect(static_cast<HddTaskExecutor*>(mExecutor),
                &HddTaskExecutor::hddPartFinished,
                this, &HddExecController::hddPartFinished);
    }
signals:
    void hddPartFinished();
};

#endif // TASKEXECUTOR_H
