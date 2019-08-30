#ifndef USESHAREDPOINTER_H
#define USESHAREDPOINTER_H
#include "smartPointers/stdpointer.h"
#include "cachecontainer.h"

template <typename T = CacheContainer>
class UseSharedPointer {
public:
    UseSharedPointer() {}

    UseSharedPointer(const stdsptr<T>& ptr) {
        setPtr(ptr);
    }

    UseSharedPointer(const UseSharedPointer<T>& other) {
        setPtr(other.mPtr);
    }

    UseSharedPointer(UseSharedPointer<T>&& other) {
        mPtr = other.mPtr;
        other.mPtr.reset();
    }

    ~UseSharedPointer() { if(mPtr) mPtr->decInUse(); }

    UseSharedPointer<T> &operator=(const UseSharedPointer<T>& other) {
        UseSharedPointer<T> tmp(other);
        swap(tmp);
        return *this;
    }

    UseSharedPointer<T> &operator=(UseSharedPointer<T> &&other) {
        UseSharedPointer<T> tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    UseSharedPointer<T> &operator=(const stdsptr<T>& t) {
        setPtr(t);
        return *this;
    }

    T* operator->() const { return mPtr.get(); }
    operator bool() const { return mPtr.get(); }
    bool operator==(T* const t) { return mPtr == t; }
    bool operator==(const stdsptr<T>& t) { return mPtr == t; }

    void swap(UseSharedPointer<T>& other) {
        std::swap(mPtr, other.mPtr);
    }

    T* get() const { return mPtr.get(); }
private:
    void setPtr(const stdsptr<T>& t) {
        if(mPtr) mPtr->decInUse();
        mPtr = t;
        if(mPtr) mPtr->incInUse();
    }

    stdsptr<T> mPtr;
};

#endif // USESHAREDPOINTER_H
