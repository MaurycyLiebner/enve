#include "taskexecutor.h"

ExecController::TaskExecutor::TaskExecutor() {}

void ExecController::TaskExecutor::processTask(_ScheduledTask* task) {
    task->_processUpdate();
    emit finishedTask(task);
}
