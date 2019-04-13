#include "taskexecutor.h"

TaskExecutor::TaskExecutor() {}

void TaskExecutor::processTask(_ScheduledTask* task) {
    task->_processUpdate();
    emit finishedTask(task);
}
