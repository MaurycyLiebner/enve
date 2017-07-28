#ifndef SELFREF_H
#define SELFREF_H
#include <QSharedPointer>

class SelfRef {
public:
    template<class T>
    QSharedPointer<T> ref() {
        if(mThisWeak.isNull()) {
            QSharedPointer<T> thisRef = QSharedPointer<T>((T*)this);
            this->mThisWeak = qSharedPointerCast<SelfRef>(thisRef).toWeakRef();
            return thisRef;
        }
        return qSharedPointerCast<T>(
                    QSharedPointer<SelfRef>(this->mThisWeak));
    }

    template<class T>
    QWeakPointer<T> weakRef() {
        return ref<T>().toWeakRef();
    }

private:
    QWeakPointer<SelfRef> mThisWeak;
};

#include <memory>

class StdSelfRef {
public:
    template<class T>
    std::shared_ptr<T> ref() {
        if(mThisWeak.expired()) {
            std::shared_ptr<T> thisRef = std::shared_ptr<T>((T*)this);
            this->mThisWeak = std::static_pointer_cast<StdSelfRef>(thisRef);
            return thisRef;
        }
        return std::static_pointer_cast<T>(
                    std::shared_ptr<StdSelfRef>(this->mThisWeak));
    }

private:
    std::weak_ptr<StdSelfRef> mThisWeak;
};

class SimpleSmartPointer {
public:
    SimpleSmartPointer() {}
    virtual ~SimpleSmartPointer() {}
    void incNumberPointers() {
        mNumberPointers++;
    }

    void decNumberPointers() {
        mNumberPointers--;
        if(mNumberPointers <= 0) {
            delete this;
        }
    }

private:
    int mNumberPointers = 0;
};

#endif // SELFREF_H
