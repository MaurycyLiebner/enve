#ifndef SELFREF_H
#define SELFREF_H
#include <QSharedPointer>
#include <QObject>


template <typename Ptr, typename List>
extern inline bool listContainsSharedPtr(Ptr ptr, List list) {
    foreach(Ptr ptrT, list) {
        if(ptrT == ptr) return true;
    }
    return false;
}

class SelfRef : public QObject {
    Q_OBJECT
public:
    template<class T>
    inline QSharedPointer<T> ref() {
        return qSharedPointerCast<T>(mThisWeak);
    }

//    template<class T>
//    inline QWeakPointer<T> weakRef() {
//        return qWeakPointerCast<T>(mThisWeak);
//    }

    template<class D, class B>
    static inline D* getAs(B* base) {
        static_assert(std::is_base_of<B, D>::value || std::is_base_of<D, B>::value, "Classes not related");
        return static_cast<D*>(base);
    }

    template<class D, class B>
    static inline D* getAs(const QPointer<B>& base) {
        static_assert(std::is_base_of<B, D>::value || std::is_base_of<D, B>::value, "Classes not related");
        return static_cast<D*>(base.data());
    }

    template<class D, class B>
    static inline QSharedPointer<D> getAs(const QSharedPointer<B>& base) {
        static_assert(std::is_base_of<B, D>::value || std::is_base_of<D, B>::value, "Classes not related");
        return base;
    }

    template <class T, typename... Args>
    static QSharedPointer<T> create(Args && ...arguments) {
        return (new T(arguments...))->template iniRef<T>();
    }
protected:
    static void *operator new (size_t);

    template<class T>
    QSharedPointer<T> iniRef() {
        Q_ASSERT_X(mThisWeak.isNull(), "SelfRef::iniRef", "reinitialization");
        QSharedPointer<T> thisRef = QSharedPointer<T>(static_cast<T*>(this));
        this->mThisWeak = qSharedPointerCast<SelfRef>(thisRef).toWeakRef();
        return thisRef;
    }
private:
    QWeakPointer<SelfRef> mThisWeak;
};

#include <memory>
template <class T>
class StdPointer;

class StdSelfRef {
public:
    virtual ~StdSelfRef();
    template<class T>
    inline std::shared_ptr<T> ref() {
        return std::static_pointer_cast<T>(mThisWeak.lock());
    }

    template<class T>
    inline std::weak_ptr<T> weakRef() {
        return mThisWeak;
    }

    template<class D, class B>
    static inline D* getAs(B* base) {
        static_assert(std::is_base_of<B, D>::value || std::is_base_of<D, B>::value, "Classes not related");
        return static_cast<D*>(base);
    }

    template<class D, class B>
    static inline D* getAs(const StdPointer<B>& base) {
        static_assert(std::is_base_of<B, D>::value || std::is_base_of<D, B>::value, "Classes not related");
        return static_cast<D*>(base.data());
    }

    template<class D, class B>
    static inline std::shared_ptr<D> getAs(const std::shared_ptr<B>& base) {
        static_assert(std::is_base_of<B, D>::value || std::is_base_of<D, B>::value, "Classes not related");
        return std::static_pointer_cast<D>(base);
    }

    template <class T, typename... Args>
    static std::shared_ptr<T> create(Args && ...arguments) {
        return (new T(arguments...))->template iniRef<T>();
    }
protected:
    static void *operator new (size_t);

    template<class T>
    std::shared_ptr<T> iniRef() {
        Q_ASSERT_X(mThisWeak.expired(), "StdSelfRef::iniRef", "reinitialization");
        std::shared_ptr<T> thisRef = std::shared_ptr<T>(static_cast<T*>(this));
        this->mThisWeak = std::static_pointer_cast<StdSelfRef>(thisRef);
        return thisRef;
    }
private:
    std::weak_ptr<StdSelfRef> mThisWeak;
};

#endif // SELFREF_H
