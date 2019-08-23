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
    void HddPartFinished();
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
        if(mCurrentTask) RuntimeThrow("Previous task did not finish yet");
        mCurrentTask = task;
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
    void finishedTask() {
        stdsptr<eTask> task;
        task.swap(mCurrentTask);
        emit finishedTaskSignal(task, this);
    }

    stdsptr<eTask> mCurrentTask;
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
                &HddTaskExecutor::HddPartFinished,
                this, &HddExecController::HddPartFinished);
    }
signals:
    void HddPartFinished();
};

#endif // TASKEXECUTOR_H
