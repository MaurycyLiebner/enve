#ifndef UPDATESCHEDULER_H
#define UPDATESCHEDULER_H

class FileCacheHandler;
class PathBox;
class Updatable;
class Gradient;

class UpdateScheduler {
public:
    UpdateScheduler() {}
    virtual ~UpdateScheduler() {}
    virtual void update() {}
    virtual bool readyToBeProcessed() { return true; }
};

class QGradientStopsUpdateScheduler : public UpdateScheduler {
public:
    QGradientStopsUpdateScheduler(Gradient *gradient);

    void update();
private:
    Gradient *mGradient;
};

#endif // UPDATESCHEDULER_H
