#ifndef MINIMALCACHECONTAINER_H
#define MINIMALCACHECONTAINER_H
#include "smartPointers/stdselfref.h"

class MinimalCacheContainer : public StdSelfRef {
protected:
    MinimalCacheContainer();
public:
    ~MinimalCacheContainer();

    virtual bool cacheAndFree() = 0;
    virtual bool freeAndRemove() = 0;

    virtual int getByteCount() = 0;

    virtual void setBlocked(const bool &bT) {
        mBlocked = bT;
    }

    bool cacheFreeAndRemoveFromMemoryHandler();

    void setHandledByMemoryHanlder(const bool &bT) {
        mHandledByMemoryHandler = bT;
    }

    const bool &handledByMemoryHandler() {
        return mHandledByMemoryHandler;
    }
protected:
    bool mHandledByMemoryHandler = false;
    bool mBlocked = false;
};

#endif // MINIMALCACHECONTAINER_H
