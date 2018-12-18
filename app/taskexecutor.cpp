#include "taskexecutor.h"

TaskExecutor::TaskExecutor(const int &id, QObject *parent) :
    QObject(parent) {
    mId = id;
}

void TaskExecutor::updateUpdatable(_ScheduledTask * const updatable,
                                     const int &targetId) {
    if(targetId == mId) {
        //auto start = std::chrono::steady_clock::now();
        //qDebug() << "started " << targetId;
        updatable->_processUpdate();
        emit finishedUpdating(mId, updatable);
        //auto end = std::chrono::steady_clock::now();
        //qDebug() << "finished " << targetId << std::chrono::duration <double, std::milli> (end - start).count() << " ms";
    }
}
