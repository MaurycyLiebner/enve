#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include "selfref.h"

class Updatable : public StdSelfRef {
public:
    Updatable();
    virtual ~Updatable();

    virtual void beforeUpdate();

    virtual void processUpdate() = 0;

    virtual void afterUpdate();

    virtual void schedulerProccessed();

    void addDependent(Updatable *updatable);

    void decDependencies() { nDependancies--; }
    void incDependencies() { nDependancies++; }


    bool readyToBeProcessed() {
        return nDependancies == 0 && !mBeingProcessed;
    }

    void addScheduler();
    virtual void addSchedulerNow();

    virtual bool shouldUpdate() {
        return true;
    }

    bool isAwaitingUpdate() { return mAwaitingUpdate; }
    bool isBeingProcessed() { return mBeingProcessed; }
    bool finished() { return mFinished; }
    bool schedulerAdded() { return mSchedulerAdded; }

    void clear();
protected:
    bool mFinished = false;
    bool mSchedulerAdded = false;
    bool mBeingProcessed = false;
    bool mAwaitingUpdate = false;

    void tellDependentThatFinished();

    std::shared_ptr<Updatable> mSelfRef;
    QList<Updatable*> mDependent;
    QList<Updatable*> mUpdateDependent;
    int nDependancies = 0;
};

#endif // UPDATABLE_H
