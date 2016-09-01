#include "updatescheduler.h"

MappedPathUpdateScheduler::MappedPathUpdateScheduler(VectorPath *path) : UpdateScheduler() {
    mPath = path;
}

void MappedPathUpdateScheduler::update() {
    mPath->updateMappedPathIfNeeded();
}

PathUpdateScheduler::PathUpdateScheduler(VectorPath *path) : UpdateScheduler() {
    mPath = path;
}

void PathUpdateScheduler::update() {
    mPath->updatePathIfNeeded();
}
