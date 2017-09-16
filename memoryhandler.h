#ifndef MEMORYHANDLER_H
#define MEMORYHANDLER_H
#include "memorychecker.h"
#include <QThread>
class MinimalCacheContainer;

class MemoryHandler : public QObject {
    Q_OBJECT
public:
    explicit MemoryHandler(QObject *parent = 0);
    ~MemoryHandler();

    void addContainer(MinimalCacheContainer *cont);
    void removeContainer(MinimalCacheContainer *cont);
    void containerUpdated(MinimalCacheContainer *cont);

    static MemoryHandler *getInstance() { return mInstance; }
signals:
    void allMemoryUsed();
    void memoryFreed();
public slots:
    void freeMemory(const unsigned long long &bytes);
private:
    static MemoryHandler *mInstance;
    QThread *mMemoryChekerThread;
    MemoryChecker *mMemoryChecker;
    QList<MinimalCacheContainer*> mContainers;
};

#endif // MEMORYHANDLER_H
