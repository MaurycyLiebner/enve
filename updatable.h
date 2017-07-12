#ifndef UPDATABLE_H
#define UPDATABLE_H
#include <QList>

class Updatable {
public:
    Updatable();

    virtual void beforeUpdate();

    virtual void processUpdate() = 0;

    virtual void afterUpdate();

    virtual void schedulerProccessed() {
        mLoadingScheduled = true;
    }


    void addDependent(Updatable *updatable);

    void decDependencies() { nDependancies--; }
    void incDependencies() { nDependancies++; }
    bool readyToBeProcessed() {
        return nDependancies == 0 && !mBeingProcessed;
    }

    void addScheduler();
private:
    bool mBeingProcessed = false;
    void tellDependentThatFinished();

    QList<Updatable*> mDependent;
    QList<Updatable*> mUpdateDependent;
    int nDependancies = 0;
    bool mLoadingScheduled = false;
};

#endif // UPDATABLE_H
