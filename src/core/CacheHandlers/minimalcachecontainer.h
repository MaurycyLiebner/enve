// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MINIMALCACHECONTAINER_H
#define MINIMALCACHECONTAINER_H
#include "smartPointers/stdselfref.h"

class CacheContainer : public StdSelfRef {
protected:
    CacheContainer();
public:
    template <typename T>
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

    ~CacheContainer();

    virtual void noDataLeft_k() = 0;
    virtual int getByteCount() = 0;
    virtual int free_RAM_k() {
        const int bytes = getByteCount();
        noDataLeft_k();
        return bytes;
    }

    void incInUse() {
        mInUse++;
        removeFromMemoryManagment();
    }

    void decInUse() {
        mInUse--;
        Q_ASSERT(mInUse >= 0);
        if(!mInUse) addToMemoryManagment();
    }

    bool handledByMemoryHandler() const {
        return mHandledByMemoryHandler;
    }

    bool inUse() const { return mInUse; }
protected:
    void addToMemoryManagment();
    void removeFromMemoryManagment();
    void updateInMemoryManagment();
private:
    bool mHandledByMemoryHandler = false;
    int mInUse = 0;
};

#endif // MINIMALCACHECONTAINER_H
