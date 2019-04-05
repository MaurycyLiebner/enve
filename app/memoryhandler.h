#ifndef MEMORYHANDLER_H
#define MEMORYHANDLER_H
#include <QThread>
#include "memorychecker.h"
class MinimalCacheContainer;

class MemoryHandler : public QObject {
    Q_OBJECT
public:
    explicit MemoryHandler(QObject * const parent = nullptr);
    ~MemoryHandler();

    void addContainer(MinimalCacheContainer * const cont);
    void removeContainer(MinimalCacheContainer * const cont);
    void containerUpdated(MinimalCacheContainer * const cont);

    static MemoryHandler *sGetInstance() { return sInstance; }
signals:
    void allMemoryUsed();
    void memoryFreed();
public slots:
    void freeMemory(const MemoryState &state,
                    const unsigned long long &minFreeBytes);
    void memoryChecked(const int& memKb, const int &totMemKb);
private:
    MemoryState mCurrentMemoryState = NORMAL_MEMORY_STATE;
    static MemoryHandler *sInstance;
    QTimer *mTimer;
    QThread *mMemoryChekerThread;
    MemoryChecker *mMemoryChecker;
    QList<MinimalCacheContainer*> mContainers;
};

#endif // MEMORYHANDLER_H
