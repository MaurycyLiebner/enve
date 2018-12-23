#ifndef STDSELFREF_H
#define STDSELFREF_H
#include <QtCore>
#include <memory>
template <class T>
class StdPointer;
template <class T> using stdsptr = std::shared_ptr<T>;

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
    static inline std::shared_ptr<T> createSPtr(Args && ...arguments) {
        return (new T(arguments...))->template iniRef<T>();
    }
protected:
    static void *operator new (size_t sz) {
        return std::malloc(sz);
    }

    template<class T>
    inline std::shared_ptr<T> ref() const {
        Q_ASSERT_X(!mThisWeak.expired(), "StdSelfRef::ref",
                   "Called before initialization");
        return std::static_pointer_cast<T>(
                    std::shared_ptr<StdSelfRef>(mThisWeak));
    }

    template<class T>
    inline std::weak_ptr<T> weakRef() const {
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
#endif // STDSELFREF_H
