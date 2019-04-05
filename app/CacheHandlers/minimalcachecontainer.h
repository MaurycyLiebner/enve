#ifndef MINIMALCACHECONTAINER_H
#define MINIMALCACHECONTAINER_H
#include "smartPointers/stdselfref.h"

class MinimalCacheContainer : public StdSelfRef {
protected:
    MinimalCacheContainer();
public:
    ~MinimalCacheContainer();

    virtual bool freeAndRemove_k() = 0;
    virtual bool freeFromMemory_k() {
        return freeAndRemove_k();
    }
    virtual int getByteCount() = 0;

    void setBlocked(const bool &bT) {
        if(bT == mBlocked) return;
        mBlocked = bT;
        if(mBlocked) removeFromMemoryManagment();
        else addToMemoryManagment();
    }

    bool handledByMemoryHandler() const {
        return mHandledByMemoryHandler;
    }

    bool blocked() const {
        return mBlocked;
    }
protected:
    void addToMemoryManagment();
    void removeFromMemoryManagment();
    void updateInMemoryManagment();
private:
    bool mHandledByMemoryHandler = false;
    bool mBlocked = false;
};

#endif // MINIMALCACHECONTAINER_H
