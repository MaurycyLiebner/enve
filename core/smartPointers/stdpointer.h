#ifndef STDPOINTER_H
#define STDPOINTER_H
#include <QtCore/QtCore>
#include "stdselfref.h"

template <class T>
class StdPointer {
    std::weak_ptr<StdSelfRef> wp;
    StdSelfRef* rp = nullptr;

    void updateRawPtr() {
        if(wp.expired()) {
            rp = nullptr;
        } else {
            std::shared_ptr<StdSelfRef> sp = wp.lock();
            rp = sp.get();
        }
    }
public:
    inline StdPointer() {
        static_assert(std::is_base_of<StdSelfRef, T>::value,
                      "StdPointer can be used only for StdSelfRef derived classes");
    }
    inline StdPointer(T *p) : wp(p->template weakRef<T>()), rp(p) {
        static_assert(std::is_base_of<StdSelfRef, T>::value,
                      "StdPointer can be used only for StdSelfRef derived classes");
    }
    // compiler-generated copy/move ctor/assignment operators are fine!
    // compiler-generated dtor is fine!

    inline void swap(StdPointer &other) {
        wp.swap(other.wp);
        updateRawPtr();
        other.updateRawPtr();
    }

    inline StdPointer<T> &operator=(T* p) {
        rp = p;
        if(p == nullptr) {
            wp.reset();
        } else {
            wp = static_cast<StdSelfRef*>(p)->weakRef<StdSelfRef>();
        }
        return *this;
    }

    inline T* data() const {
        if(wp.expired()) return nullptr;
        return static_cast<T*>(rp);
    }
    inline T* operator->() const
    { return data(); }
    inline T& operator*() const
    { return *data(); }
    inline operator T*() const
    { return data(); }

    inline bool isNull() const
    { return wp.expired(); }

    inline void clear() {
        wp.reset();
        rp = nullptr;
    }
};
template <class T> using stdptr = StdPointer<T>;

template <class T> Q_DECLARE_TYPEINFO_BODY(StdPointer<T>, Q_MOVABLE_TYPE);

template <class T>
inline bool operator==(const T *o, const StdPointer<T> &p)
{ return o == p.operator->(); }

template<class T>
inline bool operator==(const StdPointer<T> &p, const T *o)
{ return p.operator->() == o; }

template <class T>
inline bool operator==(T *o, const StdPointer<T> &p)
{ return o == p.operator->(); }

template<class T>
inline bool operator==(const StdPointer<T> &p, T *o)
{ return p.operator->() == o; }

template<class T>
inline bool operator==(const StdPointer<T> &p1, const StdPointer<T> &p2)
{ return p1.operator->() == p2.operator->(); }

template<class T1, class T2>
inline bool operator==(const StdPointer<T1> &p1, const StdPointer<T2> &p2)
{ return p1.operator->() == p2.operator->(); }

template <class T>
inline bool operator!=(const T *o, const StdPointer<T> &p)
{ return o != p.operator->(); }

template<class T>
inline bool operator!= (const StdPointer<T> &p, const T *o)
{ return p.operator->() != o; }

template <class T>
inline bool operator!=(T *o, const StdPointer<T> &p)
{ return o != p.operator->(); }

template<class T>
inline bool operator!= (const StdPointer<T> &p, T *o)
{ return p.operator->() != o; }

template<class T>
inline bool operator!= (const StdPointer<T> &p1, const StdPointer<T> &p2)
{ return p1.operator->() != p2.operator->() ; }


#endif // STDPOINTER_H
