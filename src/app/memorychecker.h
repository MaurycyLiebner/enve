#ifndef MEMORYCHECKER_H
#define MEMORYCHECKER_H

#include <QObject>
#include <QTimer>

enum MemoryState {
    NORMAL_MEMORY_STATE,
    LOW_MEMORY_STATE = 5,
    VERY_LOW_MEMORY_STATE = 15,
    CRITICAL_MEMORY_STATE = 30
};

class MemoryChecker : public QObject {
    Q_OBJECT
public:
    explicit MemoryChecker(QObject * const parent = nullptr);
    static MemoryChecker *getInstance() { return mInstance; }

    void checkMemory();
private:
    static long sGetFreeBytes();
    static char sLine[256];

    MemoryState mLastMemoryState = NORMAL_MEMORY_STATE;

    long mLowFreeBytes = 0;
    long mVeryLowFreeBytes = 0;

    static MemoryChecker *mInstance;
signals:
    void memoryCheckedKB(int, int);
    void handleMemoryState(MemoryState, long bytesToFree);
};

#endif // MEMORYCHECKER_H
