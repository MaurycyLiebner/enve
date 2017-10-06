#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>
#include "selfref.h"
#include <QEventLoop>
class PaintControler;

class Executable : public StdSelfRef {
public:
    Executable() {}
    virtual ~Executable() {}
    void setCurrentPaintControler(PaintControler *paintControler) {
        mCurrentPaintControler = paintControler;
    }

    virtual void beforeUpdate() {
        mSelfRef = ref<Executable>();
        mBeingProcessed = true;
    }

    virtual void processUpdate() = 0;

    void updateFinished() {
        mFinished = true;
        mBeingProcessed = false;
        mCurrentPaintControler = NULL;
        afterUpdate();
        mSelfRef.reset();
    }

    virtual void afterUpdate() {

    }

    bool isBeingProcessed() { return mBeingProcessed; }
    bool finished() { return mFinished; }

    void waitTillProcessed();

    virtual bool readyToBeProcessed() {
        return !mBeingProcessed;
    }

    virtual void clear() {
        mFinished = false;
        mBeingProcessed = false;
        mSelfRef.reset();
    }
protected:
    PaintControler *mCurrentPaintControler = NULL;
    bool mFinished = false;
    bool mBeingProcessed = false;
    std::shared_ptr<Executable> mSelfRef;
};

class Updatable : public Executable {
public:
    Updatable() {
        mFinished = true;
    }
    ~Updatable();

    void beforeUpdate();

    void afterUpdate();

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
    bool schedulerAdded() { return mSchedulerAdded; }

    void clear();
    virtual bool isFileUpdatable() { return false; }
protected:
    bool mSchedulerAdded = false;
    bool mAwaitingUpdate = false;

    void tellDependentThatFinished();

    QList<Updatable*> mDependent;
    QList<Updatable*> mUpdateDependent;
    int nDependancies = 0;
};

#endif // UPDATABLE_H
