#ifndef SELFREF_H
#define SELFREF_H
#include <QPointer>
#include <QSharedPointer>
#include <QObject>
#include "../exceptions.h"
#include "eobject.h"

template <class T> using qsptr = QSharedPointer<T>;
template <class T> using qptr = QPointer<T>;

class SelfRef : public QObject {
    e_PROHIBIT_HEAP
public:
    template <class T, typename... Args>
    static inline QSharedPointer<T> sCreate(Args && ...args) {
        return (new T(std::forward<Args>(args)...))->template iniRef<T>();
    }

    template<class T>
    inline QSharedPointer<T> ref() {
        if(mThisWeak.isNull()) RuntimeThrow("Not initialized, or already deleting");
        return qSharedPointerCast<T>(mThisWeak);
    }
private:
    template<class T>
    QSharedPointer<T> iniRef() {
        Q_ASSERT_X(mThisWeak.isNull(), "SelfRef::iniRef", "reinitialization");
        QSharedPointer<T> thisRef = QSharedPointer<T>(static_cast<T*>(this));
        this->mThisWeak = qSharedPointerCast<SelfRef>(thisRef).toWeakRef();
        return thisRef;
    }

    template<class T>
    inline QWeakPointer<T> weakRef() {
        if(mThisWeak.isNull()) return QWeakPointer<T>();
        return qWeakPointerCast<T>(mThisWeak);
    }
private:
    QWeakPointer<SelfRef> mThisWeak;
};

#endif // SELFREF_H
