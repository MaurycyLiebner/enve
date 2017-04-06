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
    unsigned long mFreeRam;
    unsigned long mTotalRam;
    unsigned int mMemUnit;
private slots:
    void checkMemory();
signals:
    void freeMemory(unsigned long);
public slots:
};

#endif // MEMORYCHECKER_H
