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
        if(base == nullptr) return nullptr;
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
    static inline QSharedPointer<T> create(Args && ...arguments) {
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
        Q_ASSERT_X(!mThisWeak.isNull(), "SelfRef::ref",
                   "Called before initialization");
        return qSharedPointerCast<T>(mThisWeak);
    }

    template<class T>
    inline QWeakPointer<T> weakRef() {
        Q_ASSERT_X(!mThisWeak.isNull(), "SelfRef::weakRef",
                   "Called before initialization");
        return qWeakPointerCast<T>(mThisWeak);
    }
private:
    QWeakPointer<SelfRef> mThisWeak;
};

#include <memory>
template <class T>
class StdPointer;
template <class T> using sptr = std::shared_ptr<T>;
class StdSelfRef {
    template <class T> friend class StdPointer;
public:
    virtual ~StdSelfRef();

    template<class D, class B>
    static inline D* getAsPtr(B* base) {
        static_assert(std::is_base_of<B, D>::value ||
                      std::is_base_of<D, B>::value, "Classes not related");
        return static_cast<D*>(base);
    }

    template<class D, class B>
    static inline D* getAsPtr(const StdPointer<B>& base) {
        return getAsPtr<D>(base.data());
    }

    template<class D, class B>
    static inline D* getAsPtr(const std::shared_ptr<B>& base) {
        static_assert(std::is_base_of<B, D>::value ||
                      std::is_base_of<D, B>::value, "Classes not related");
        return getAsPtr<D>(base.get());
    }

    template<class D, class B>
    static inline std::shared_ptr<D> getAsSPtr(B* base) {
        static_assert(std::is_base_of<B, D>::value ||
                      std::is_base_of<D, B>::value, "Classes not related");
        if(base == nullptr) return nullptr;
        return base->template ref<D>();
    }

    template<class D, class B>
    static inline std::shared_ptr<D> getAsSPtr(const StdPointer<B>& base) {
        return getAsSPtr<D>(base.data());
    }

    template<class D, class B>
    static inline std::shared_ptr<D> getAsSPtr(const std::shared_ptr<B>& base) {
        static_assert(std::is_base_of<B, D>::value ||
                      std::is_base_of<D, B>::value, "Classes not related");
        return std::static_pointer_cast<D>(base);
    }

    template <class T, typename... Args>
    static inline std::shared_ptr<T> create(Args && ...arguments) {
        return (new T(arguments...))->template iniRef<T>();
    }
protected:
    static void *operator new (size_t sz) {
        return std::malloc(sz);
    }

    template<class T>
    inline std::shared_ptr<T> ref() {
        Q_ASSERT_X(!mThisWeak.expired(), "StdSelfRef::ref",
                   "Called before initialization");
        return std::static_pointer_cast<T>(
                    std::shared_ptr<StdSelfRef>(mThisWeak));
    }

    template<class T>
    inline std::weak_ptr<T> weakRef() {
        return ref<T>();
    }

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
