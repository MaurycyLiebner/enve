#ifndef MINIMALCACHECONTAINER_H
#define MINIMALCACHECONTAINER_H
#include "smartPointers/stdselfref.h"

class CacheContainer : public StdSelfRef {
protected:
    CacheContainer();
public:
    ~CacheContainer();

    virtual void noDataLeft_k() = 0;
    virtual int getByteCount() = 0;
    virtual int free_RAM_k() {
        const int bytes = getByteCount();
        noDataLeft_k();
        return bytes;
    }

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
