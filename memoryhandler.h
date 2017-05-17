#ifndef MEMORYHANDLER_H
#define MEMORYHANDLER_H
#include "memorychecker.h"
#include <QThread>
class CacheContainer;

class MemoryHandler : public QObject {
    Q_OBJECT
public:
    explicit MemoryHandler(QObject *parent = 0);
    ~MemoryHandler();

    void addContainer(CacheContainer *cont);
    void removeContainer(CacheContainer *cont);
    void containerUpdated(CacheContainer *cont);

    static MemoryHandler *getInstance() { return mInstance; }
signals:
    void allMemoryUsed();
public slots:
    void freeMemory(const unsigned long long &bytes);
private:
    static MemoryHandler *mInstance;
    QThread *mMemoryChekerThread;
    MemoryChecker *mMemoryChecker;
    QList<CacheContainer*> mContainers;
};

#endif // MEMORYHANDLER_H
