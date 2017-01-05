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

class AwaitUpdateUpdateScheduler : public UpdateScheduler {
public:
    AwaitUpdateUpdateScheduler(BoundingBox *target);

    void update();
private:
    BoundingBox *mTarget;
};

#endif // UPDATESCHEDULER_H
