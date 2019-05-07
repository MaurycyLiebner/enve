#ifndef TASKEXECUTOR_H
#define TASKEXECUTOR_H
#include <QObject>
#include <QTimer>
#include <QThread>
#include "updatable.h"

class TaskExecutor : public QObject {
    Q_OBJECT
public:
    explicit TaskExecutor();
signals:
    void finishedTask(Task*);
public slots:
    void processTask(Task* task);
};

class ExecController : public QObject {
    Q_OBJECT
public:
    ExecController(QObject * const parent = nullptr) : QObject(parent),
        mExecutor(new TaskExecutor),
        mExecutorThread(new QThread(this)) {
        connect(this, &ExecController::processTaskSignal,
                mExecutor, &TaskExecutor::processTask);
        connect(mExecutor, &TaskExecutor::finishedTask,
                this, &ExecController::finishedTask);
        mExecutor->moveToThread(mExecutorThread);
        mExecutorThread->start();
    }

    void processTask(const stdsptr<Task>& task) {
        if(mCurrentTask) Q_ASSERT(false);//RuntimeThrow("Previous task did not finish yet");
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
    void processTaskSignal(Task*);
    void finishedTaskSignal(stdsptr<Task>, ExecController*);
private:
    void finishedTask() {
        stdsptr<Task> task;
        task.swap(mCurrentTask);
        emit finishedTaskSignal(task, this);
    }

    stdsptr<Task> mCurrentTask;
    TaskExecutor * const mExecutor;
    QThread * const mExecutorThread;
};

#endif // TASKEXECUTOR_H
