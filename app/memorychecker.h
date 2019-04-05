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
    explicit MemoryChecker(QObject * const parent = nullptr);
    static MemoryChecker *getInstance() { return mInstance; }
private:
    void setCurrentMemoryState(const MemoryState &state);

    MemoryState mCurrentMemoryState = NORMAL_MEMORY_STATE;

    unsigned long long mTotalRam = 0;
    unsigned long long mLowFreeRam = 0;
    unsigned long long mVeryLowFreeRam = 0;
    QTimer *mTimer;

    static MemoryChecker *mInstance;
public slots:
    void checkMemory();
signals:
    void memoryChecked(const int&, const int&);
    void handleMemoryState(const MemoryState &,
                           const unsigned long long &);
};

#endif // MEMORYCHECKER_H
