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

extern unsigned long long getFreeRam();
class MemoryChecker : public QObject {
    Q_OBJECT
public:
    explicit MemoryChecker(QObject *parent = 0);
    static MemoryChecker *getInstance() { return mInstance; }
//    void decUsedMemory(const unsigned long long &used) {
//        mUsedRam -= used;
//    }

//    void incUsedMemory(const unsigned long long &used) {
//        mUsedRam += used;
//    }
    unsigned long long getMinFreeRam() {
        return mMinFreeRam;
    }

    void setCurrentMemoryState(const MemoryState &state);
private:
    MemoryState mCurrentMemoryState = NORMAL_MEMORY_STATE;
    QTimer *mTimer;

    unsigned long long mMinFreeRam = 0ULL;
//    unsigned long long mUsedRam = 0ULL;
//    unsigned long long mLeaveUnused = 1500000000ULL;

//    unsigned long long mFreeRam;
//    unsigned long long mTotalRam;
//    unsigned long long mMemUnit;
    static MemoryChecker *mInstance;
    QList<int> mPgFltSamples;
    int mLastPgFlts = -1;
private slots:
    void checkMemory();
    void checkMajorMemoryPageFault();
signals:
    void handleMemoryState(const MemoryState &,
                           const unsigned long long &);
};

#endif // MEMORYCHECKER_H
