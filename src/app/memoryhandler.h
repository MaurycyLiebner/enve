#ifndef MEMORYHANDLER_H
#define MEMORYHANDLER_H
#include <QThread>
#include "memorychecker.h"
#include "memorydatahandler.h"

class MemoryHandler : public QObject {
    Q_OBJECT
public:
    explicit MemoryHandler(QObject * const parent = nullptr);
    ~MemoryHandler();

    static MemoryHandler *sInstance;
signals:
    void allMemoryUsed();
    void memoryFreed();
private:
    void freeMemory(const MemoryState &state, const long &minFreeBytes);
    void memoryChecked(const int memKb, const int totMemKb);

    MemoryDataHandler mDataHandler;
    MemoryState mCurrentMemoryState = NORMAL_MEMORY_STATE;
    QTimer *mTimer;
    QThread *mMemoryChekerThread;
    MemoryChecker *mMemoryChecker;
};

#endif // MEMORYHANDLER_H
