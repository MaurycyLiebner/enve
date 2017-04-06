#ifndef MEMORYHANDLER_H
#define MEMORYHANDLER_H
#include "memorychecker.h"
#include <QThread>
class CacheBoundingBoxRenderContainer;

class MemoryHandler : public QObject {
    Q_OBJECT
public:
    explicit MemoryHandler(QObject *parent = 0);
    ~MemoryHandler();

    void addContainer(CacheBoundingBoxRenderContainer *cont);
    void removeContainer(CacheBoundingBoxRenderContainer *cont);
    void containerUpdated(CacheBoundingBoxRenderContainer *cont);

    static MemoryHandler *getInstance() { return mInstance; }
signals:

public slots:
    void freeMemory(const ulong &bytes);
private:
    static MemoryHandler *mInstance;
    QThread *mMemoryChekerThread;
    MemoryChecker *mMemoryChecker;
    QList<CacheBoundingBoxRenderContainer*> mContainers;
};

#endif // MEMORYHANDLER_H
