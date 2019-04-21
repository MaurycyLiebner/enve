#include "taskexecutor.h"

TaskExecutor::TaskExecutor() {}

void TaskExecutor::processTask(Task* task) {
    try {
        task->_processUpdate();
    } catch(...) {
        task->setException(std::current_exception());
    }
    emit finishedTask(task);
}
