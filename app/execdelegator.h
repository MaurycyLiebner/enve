#ifndef EXECDELEGATOR_H
#define EXECDELEGATOR_H
#include <QtConcurrent>
#include <QFuture>

class ExecDelegator {
public:
    ExecDelegator();

    void run() {
        mFirstAccess = true;
        mFuture = QtConcurrent::run(this, &ExecDelegator::_exec);
    }

    void access() {
        if(mFirstAccess) {
            mFuture.result();
            mFirstAccess = false;
            whenAccessed();
        }
    }

    // other thread
    virtual bool _exec() = 0;
protected:
    virtual void whenAccessed() = 0;
    bool mFirstAccess = false;
    QFuture<bool> mFuture;
};

//class DelegatorPool {
//public:

//private:
//    QList<ExecDelegator*> mCurrentDelegators;
//}

#endif // EXECDELEGATOR_H
