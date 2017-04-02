#ifndef UPDATESCHEDULER_H
#define UPDATESCHEDULER_H

#include "Boxes/vectorpath.h"
#include "canvas.h"
#include <QDebug>

class UpdateScheduler {
public:
    UpdateScheduler() {}
    virtual ~UpdateScheduler() {}
    virtual void update() {}
};

class QGradientStopsUpdateScheduler : public UpdateScheduler {
public:
    QGradientStopsUpdateScheduler(Gradient *gradient);

    void update();
private:
    Gradient *mGradient;
};

class AddBoxAwaitingUpdateScheduler : public UpdateScheduler {
public:
    AddBoxAwaitingUpdateScheduler(Canvas *canvas);

    void update();
private:
    Canvas *mCanvas;
};

class PathUpdateScheduler : public UpdateScheduler {
public:
    PathUpdateScheduler(PathBox *path);

    void update();
private:
    PathBox *mPath;
};

class ApplyRenderCacheChangesScheduler : public UpdateScheduler {
public:
    ApplyRenderCacheChangesScheduler(BoundingBox *box);

    void update();
private:
    BoundingBox *mTarget;
};

#endif // UPDATESCHEDULER_H
