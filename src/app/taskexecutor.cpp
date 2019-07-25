#include "taskexecutor.h"

void TaskExecutor::processTask(Task* task) {
    try {
        task->process();
    } catch(...) {
        task->setException(std::current_exception());
    }
    emit finishedTask(task);
}
