#ifndef QATOMICLIST_H
#define QATOMICLIST_H

#include <QList>
#include <QMutex>
#include <QMutexLocker>

template <typename T>
class QAtomicList : private QList<T> {
public:
    class Locker {
    public:
        Locker() {}
        ~Locker() { if(mMutex) mMutex->unlock(); }
    private:
        void lock(QMutex& mutex) {
            mMutex = &mutex;
            mutex.lock();
        }

        QMutex *mMutex = nullptr;
    };

    QList<T>& lock(Locker& locker) {
        locker.lock(*this);
        return *this;
    }

    void copy(QList<T>& list) {
        Locker locker;
        list = lock(locker);
    }

    void append(const T& t) {
        Locker locker;
        lock(locker).append(t);
    }

    bool takeFirst(T& t) {
        Locker locker;
        auto& list = lock(locker);
        if(list.isEmpty()) return false;
        t = list.takeFirst();
        return true;
    }
private:
    QMutex mMutex;
};

#endif // QATOMICLIST_H
