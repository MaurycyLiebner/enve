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
    explicit MemoryChecker(QObject *parent = nullptr);
    static MemoryChecker *getInstance() { return mInstance; }
//    void decUsedMemory(const unsigned long long &used) {
//        mUsedRam -= used;
//    }

//    void incUsedMemory(const unsigned long long &used) {
//        mUsedRam += used;
//    }
    unsigned long long getLowFreeRam() {
        return mLowFreeRam;
    }

    void setCurrentMemoryState(const MemoryState &state);
private:
    MemoryState mCurrentMemoryState = NORMAL_MEMORY_STATE;
    int mLastPgFlts = -1;

    unsigned long long mTotalRam = 0ULL;
    unsigned long long mLowFreeRam = 0ULL;
    unsigned long long mVeryLowFreeRam = 0ULL;
    QTimer *mTimer;

//    unsigned long long mUsedRam = 0ULL;
//    unsigned long long mLeaveUnused = 1500000000ULL;

//    unsigned long long mFreeRam;
//    unsigned long long mTotalRam;
//    unsigned long long mMemUnit;
    static MemoryChecker *mInstance;
    QList<int> mPgFltSamples;
private slots:
    void checkMemory();
    void checkMajorMemoryPageFault();
signals:
    void memoryChecked(const int&, const int&);
    void handleMemoryState(const MemoryState &,
                           const unsigned long long &);
};

#endif // MEMORYCHECKER_H
