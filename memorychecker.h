#ifndef MEMORYCHECKER_H
#define MEMORYCHECKER_H

#include <QObject>
#include <QTimer>

class MemoryChecker : public QObject {
    Q_OBJECT
public:
    explicit MemoryChecker(QObject *parent = 0);

private:
    QTimer *mTimer;
    unsigned long long mFreeRam;
    unsigned long long mTotalRam;
    unsigned long long mMemUnit;
private slots:
    void checkMemory();
signals:
    void freeMemory(unsigned long long);
public slots:
};

#endif // MEMORYCHECKER_H
