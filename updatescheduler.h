#ifndef UPDATESCHEDULER_H
#define UPDATESCHEDULER_H

#include "vectorpath.h"
#include "canvas.h"
#include <QDebug>

class UpdateScheduler {
public:
    UpdateScheduler() {}
    virtual ~UpdateScheduler() {}
    virtual void update() { }
};

class PathUpdateScheduler : public UpdateScheduler {
public:
    PathUpdateScheduler(VectorPath *path);

    void update();
private:
    VectorPath *mPath;
};

class MappedPathUpdateScheduler : public UpdateScheduler {
public:
    MappedPathUpdateScheduler(VectorPath *path);

    void update();
private:
    VectorPath *mPath;
};

class OutlineUpdateScheduler : public UpdateScheduler {
public:
    OutlineUpdateScheduler(VectorPath *path);

    void update();

private:
    VectorPath *mPath;
};

class QGradientStopsUpdateScheduler : public UpdateScheduler {
public:
    QGradientStopsUpdateScheduler(Gradient *gradient);

    void update();
private:
    Gradient *mGradient;
};

#endif // UPDATESCHEDULER_H
