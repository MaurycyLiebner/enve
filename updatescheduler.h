#ifndef UPDATESCHEDULER_H
#define UPDATESCHEDULER_H

class FileCacheHandler;
class PathBox;
class _ScheduledExecutor;
class Gradient;

class UpdateScheduler {
public:
    UpdateScheduler() {}
    virtual ~UpdateScheduler() {}
    virtual void update() {}
};

#endif // UPDATESCHEDULER_H
