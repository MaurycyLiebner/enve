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
    void finishedTask(_ScheduledTask*);
public slots:
    void processTask(_ScheduledTask* task);
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

    void processTask(_ScheduledTask * task) {
        emit processTaskSignal(task);
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
    void processTaskSignal(_ScheduledTask*);
    void finishedTaskSignal(_ScheduledTask*, ExecController*);
private slots:
    void finishedTask(_ScheduledTask* task) {
        emit finishedTaskSignal(task, this);
    }
private:
    TaskExecutor * const mExecutor;
    QThread * const mExecutorThread;
};

#endif // TASKEXECUTOR_H
