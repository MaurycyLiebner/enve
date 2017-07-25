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

class AddUpdatableAwaitingUpdateScheduler : public UpdateScheduler {
public:
    AddUpdatableAwaitingUpdateScheduler(Updatable *updatable);

    void update();
private:
    Updatable *mUpdatable;
};
#endif // UPDATESCHEDULER_H
