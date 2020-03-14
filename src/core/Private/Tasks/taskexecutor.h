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
    void processTask(const stdsptr<eTask>* task);
signals:
    void finishedTask(const stdsptr<eTask>*);
};

class HddTaskExecutor : public TaskExecutor {
    Q_OBJECT
public:
    explicit HddTaskExecutor() {}
signals:
    void hddPartFinished();
};

class ExecController : public QThread {
    Q_OBJECT
protected:
    ExecController(TaskExecutor * const executor,
                   QObject * const parent = nullptr) : QThread(parent),
        mExecutor(executor) {
        connect(this, &ExecController::processTaskSignal,
                mExecutor, &TaskExecutor::processTask,
                Qt::QueuedConnection);
        connect(mExecutor, &TaskExecutor::finishedTask,
                this, &ExecController::finishedTask,
                Qt::QueuedConnection);
        mExecutor->moveToThread(this);
        start();
    }
public:
    void processTask(const stdsptr<eTask>& task) {
        mProcessing++;
        emit processTaskSignal(new stdsptr<eTask>(task));
    }

    bool finished() const { return mProcessing == 0; }
signals:
    void processTaskSignal(const stdsptr<eTask>*);
    void finishedTaskSignal(const stdsptr<eTask>&, ExecController*);
protected:
    TaskExecutor * const mExecutor;
private:
    void finishedTask(const stdsptr<eTask>* task) {
        mProcessing--;
        emit finishedTaskSignal(*task, this);
        delete task;
    }

    int mProcessing = 0;
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
                this, &HddExecController::hddPartFinished,
                Qt::QueuedConnection);
    }
signals:
    void hddPartFinished();
};

#endif // TASKEXECUTOR_H
