#ifndef MEMORYHANDLER_H
#define MEMORYHANDLER_H
#include <QThread>
#include "memorychecker.h"
class MinimalCacheContainer;

class MemoryHandler : public QObject {
    Q_OBJECT
public:
    explicit MemoryHandler(QObject *parent = nullptr);
    ~MemoryHandler();

    void addContainer(MinimalCacheContainer *cont);
    void removeContainer(MinimalCacheContainer *cont);
    void containerUpdated(MinimalCacheContainer *cont);

    static MemoryHandler *getInstance() { return mInstance; }
    void incMemoryScheduledToRemove(const int &mem) {
        mMemoryScheduledToRemove += mem;
    }
signals:
    void allMemoryUsed();
    void memoryFreed();
public slots:
    void freeMemory(const MemoryState &state,
                    const unsigned long long &minFreeBytes);
    void memoryChecked(const int& memKb, const int &totMemKb);
private:
    MemoryState mCurrentMemoryState = NORMAL_MEMORY_STATE;
    long long mMemoryScheduledToRemove = 0;
    static MemoryHandler *mInstance;
    QTimer *mTimer;
    QThread *mMemoryChekerThread;
    MemoryChecker *mMemoryChecker;
    QList<MinimalCacheContainer*> mContainers;
};

#endif // MEMORYHANDLER_H
