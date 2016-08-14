#ifndef UPDATESCHEDULER_H
#define UPDATESCHEDULER_H

#include "vectorpath.h"
#include "canvas.h"

class UpdateScheduler {
public:
    UpdateScheduler() {}
    virtual void update() {}
};

class PathUpdateScheduler : public UpdateScheduler {
public:
    PathUpdateScheduler(VectorPath *path) : UpdateScheduler() {
        mPath = path;
    }

    void update() {
        mPath->updatePathIfNeeded();
    }
private:
    VectorPath *mPath;
};

class MappedPathUpdateScheduler : public UpdateScheduler {
public:
    MappedPathUpdateScheduler(VectorPath *path) : UpdateScheduler() {
        mPath = path;
    }

    void update() {
        mPath->updateMappedPathIfNeeded();
    }
private:
    VectorPath *mPath;
};

#endif // UPDATESCHEDULER_H
