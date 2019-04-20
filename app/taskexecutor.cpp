#include "taskexecutor.h"

TaskExecutor::TaskExecutor() {}

void TaskExecutor::processTask(_ScheduledTask* task) {
    try {
        task->_processUpdate();
    } catch(...) {
        task->setException(std::current_exception());
    }
    emit finishedTask(task);
}
