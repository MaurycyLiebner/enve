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

class PathUpdateScheduler : public UpdateScheduler {
public:
    PathUpdateScheduler(PathBox *path);

    void update();
private:
    PathBox *mPath;
};

class FileCacheLoadScheduler : public UpdateScheduler {
public:
    FileCacheLoadScheduler(FileCacheHandler *cacheHandler);

    void update();
private:
    FileCacheHandler *mHandler;
};

#endif // UPDATESCHEDULER_H
