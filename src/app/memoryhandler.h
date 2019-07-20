#ifndef MEMORYHANDLER_H
#define MEMORYHANDLER_H
#include <QThread>
#include "memorychecker.h"
class CacheContainer;

class MemoryHandler : public QObject {
    Q_OBJECT
public:
    explicit MemoryHandler(QObject * const parent = nullptr);
    ~MemoryHandler();

    void addContainer(CacheContainer * const cont);
    void removeContainer(CacheContainer * const cont);
    void containerUpdated(CacheContainer * const cont);

    static MemoryHandler *sGetInstance() { return sInstance; }
signals:
    void allMemoryUsed();
    void memoryFreed();
public slots:
    void freeMemory(const MemoryState &state,
                    const unsigned long long &minFreeBytes);
    void memoryChecked(const int memKb, const int totMemKb);
private:
    MemoryState mCurrentMemoryState = NORMAL_MEMORY_STATE;
    static MemoryHandler *sInstance;
    QTimer *mTimer;
    QThread *mMemoryChekerThread;
    MemoryChecker *mMemoryChecker;
    QList<CacheContainer*> mContainers;
};

#endif // MEMORYHANDLER_H
