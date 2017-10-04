#ifndef MEMORYCHECKER_H
#define MEMORYCHECKER_H

#include <QObject>
#include <QTimer>

extern unsigned long long getFreeRam();
class MemoryChecker : public QObject {
    Q_OBJECT
public:
    explicit MemoryChecker(QObject *parent = 0);
    static MemoryChecker *getInstance() { return mInstance; }
    void setMemoryReleaseSlowedDown(const bool &bT);
//    void decUsedMemory(const unsigned long long &used) {
//        mUsedRam -= used;
//    }

//    void incUsedMemory(const unsigned long long &used) {
//        mUsedRam += used;
//    }
    unsigned long long getMinFreeRam() {
        return mMinFreeRam;
    }
private:
    bool mSlowedDown = false;
    QTimer *mTimer;

    unsigned long long mSlowMinFreeRam = 0ULL;
    unsigned long long mMinFreeRam = 0ULL;
//    unsigned long long mUsedRam = 0ULL;
//    unsigned long long mLeaveUnused = 1500000000ULL;

//    unsigned long long mFreeRam;
//    unsigned long long mTotalRam;
//    unsigned long long mMemUnit;
    static MemoryChecker *mInstance;
private slots:
    void checkMemory();
signals:
    void outOfMemory(unsigned long long);

};

#endif // MEMORYCHECKER_H
