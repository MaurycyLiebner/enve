#ifndef SELFREF_H
#define SELFREF_H
#include <QPointer>
#include <QSharedPointer>
#include <QObject>
#include "../exceptions.h"

template <class T> using qsptr = QSharedPointer<T>;
template <class T> using qptr = QPointer<T>;
class SelfRef : public QObject {
public:
    template<class D, class B>
    static inline D* getAsPtr(B* base) {
        static_assert(std::is_base_of<B, D>::value ||
                      std::is_base_of<D, B>::value, "Classes not related");
        return static_cast<D*>(base);
    }

    template<class D, class B>
    static inline D* getAsPtr(const QPointer<B>& base) {
        return getAsPtr<D>(base.data());
    }

    template<class D, class B>
    static inline D* getAsPtr(const QSharedPointer<B>& base) {
        return getAsPtr<D>(base.data());
    }

    template<class D, class B>
    static inline QSharedPointer<D> getAsSPtr(B* base) {
        static_assert(std::is_base_of<B, D>::value ||
                      std::is_base_of<D, B>::value, "Classes not related");
        if(!base) return nullptr;
        return base->template ref<D>();
    }

    template<class D, class B>
    static inline QSharedPointer<D> getAsSPtr(const QPointer<B>& base) {
        return getAsSPtr<D>(base.data());
    }

    template<class D, class B>
    static inline QSharedPointer<D> getAsSPtr(const QSharedPointer<B>& base) {
        static_assert(std::is_base_of<B, D>::value ||
                      std::is_base_of<D, B>::value, "Classes not related");
        return qSharedPointerCast<D>(base);
    }

    template <class T, typename... Args>
    static inline QSharedPointer<T> createSPtr(Args && ...arguments) {
        return (new T(arguments...))->template iniRef<T>();
    }
protected:
    static void *operator new (size_t sz) {
        return std::malloc(sz);
    }

    template<class T>
    QSharedPointer<T> iniRef() {
        Q_ASSERT_X(mThisWeak.isNull(), "SelfRef::iniRef", "reinitialization");
        QSharedPointer<T> thisRef = QSharedPointer<T>(static_cast<T*>(this));
        this->mThisWeak = qSharedPointerCast<SelfRef>(thisRef).toWeakRef();
        return thisRef;
    }

    template<class T>
    inline QSharedPointer<T> ref() {
        if(mThisWeak.isNull())
            RuntimeThrow("Not initialized, or already deleting");
        return qSharedPointerCast<T>(mThisWeak);
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
