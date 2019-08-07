#ifndef STDSELFREF_H
#define STDSELFREF_H
#include <QtCore>
#include <memory>
#include "../exceptions.h"
#include "eobject.h"

template <class T> class StdPointer;
template <class T> using stdsptr = std::shared_ptr<T>;

class StdSelfRef {
    template <class T> friend class StdPointer;
    e_PROHIBIT_HEAP
public:
    virtual ~StdSelfRef();

    template <class T, typename... Args>
    static inline std::shared_ptr<T> sCreate(Args && ...args) {
        return (new T(std::forward<Args>(args)...))->template iniRef<T>();
    }

    template<class T>
    inline std::shared_ptr<T> ref() const {
        if(mThisWeak.expired()) RuntimeThrow("Not initialized, or already deleting");
        return std::static_pointer_cast<T>(std::shared_ptr<StdSelfRef>(mThisWeak));
    }
private:
    template<class T>
    inline std::weak_ptr<T> weakRef() const {
        if(mThisWeak.expired()) return std::weak_ptr<T>();
        return ref<T>();
    }

    template<class T>
    std::shared_ptr<T> iniRef() {
        if(!mThisWeak.expired()) RuntimeThrow("Shared pointer reinitialization");
        std::shared_ptr<T> thisRef = std::shared_ptr<T>(static_cast<T*>(this));
        this->mThisWeak = std::static_pointer_cast<StdSelfRef>(thisRef);
        return thisRef;
    }
private:
    std::weak_ptr<StdSelfRef> mThisWeak;
};
#endif // STDSELFREF_H
