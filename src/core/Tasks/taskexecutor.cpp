#include "taskexecutor.h"

void TaskExecutor::processTask(eTask* task) {
    try {
        task->process();
    } catch(...) {
        task->setException(std::current_exception());
    }
    emit finishedTask(task);
}
