#ifndef MEMORYDATAHANDLER_H
#define MEMORYDATAHANDLER_H
#include <QList>

class CacheContainer;

class MemoryDataHandler {
public:
    MemoryDataHandler();

    static MemoryDataHandler *sInstance;

    void addContainer(CacheContainer * const cont);
    void removeContainer(CacheContainer * const cont);
    void containerUpdated(CacheContainer * const cont);

    bool isEmpty() const { return mContainers.isEmpty(); }
    CacheContainer* takeFirst() {
        return mContainers.takeFirst();
    }
private:
    QList<CacheContainer*> mContainers;
};

#endif // MEMORYDATAHANDLER_H
