#ifndef USEPOINTER_H
#define USEPOINTER_H
#include "smartPointers/stdpointer.h"
#include "cachecontainer.h"

template <typename T = CacheContainer>
class UsePointer {
public:
    UsePointer() {}

    UsePointer(T* const ptr) {
        setPtr(ptr);
    }

    UsePointer(const UsePointer<T>& other) {
        setPtr(other.get());
    }

    UsePointer(UsePointer<T>&& other) {
        mPtr = other.mPtr;
        other.mPtr = nullptr;
    }

    ~UsePointer() { if(mPtr) mPtr->decInUse(); }

    UsePointer<T> &operator=(const UsePointer<T>& other) {
        UsePointer<T> tmp(other);
        swap(tmp);
        return *this;
    }

    UsePointer<T> &operator=(UsePointer<T> &&other) {
        UsePointer<T> tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    UsePointer<T> &operator=(T* const t) {
        setPtr(t);
        return *this;
    }

    T* operator->() const { return mPtr; }
    operator bool() const { return mPtr; }
    bool operator==(T* const t) { return mPtr == t; }

    void swap(UsePointer<T>& other) {
        std::swap(mPtr, other.mPtr);
    }

    T* get() const { return mPtr; }
private:
    void setPtr(T* const t) {
        if(mPtr) mPtr->decInUse();
        mPtr = t;
        if(mPtr) mPtr->incInUse();
    }

    stdptr<T> mPtr = nullptr;
};

#endif // USEPOINTER_H
