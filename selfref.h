#ifndef SELFREF_H
#define SELFREF_H
#include <QSharedPointer>
#include <QObject>

template <typename Ptr, typename List>
extern bool listContainsSharedPtr(Ptr ptr, List list) {
    foreach(Ptr ptrT, list) {
        if(ptrT == ptr) return true;
    }
    return false;
}

class SelfRef : public QObject {
    Q_OBJECT
public:
    template<class T>
    QSharedPointer<T> ref() {
        if(mThisWeak.isNull()) {
            QSharedPointer<T> thisRef;
            if(mTmpPtr.isNull()) {
                thisRef = QSharedPointer<T>((T*)this);
            } else {
                thisRef = qSharedPointerCast<T>(mTmpPtr);
                mTmpPtr.reset();
            }
            this->mThisWeak = qSharedPointerCast<SelfRef>(thisRef).toWeakRef();
            return thisRef;
        }
        return qSharedPointerCast<T>(
                    QSharedPointer<SelfRef>(this->mThisWeak));
    }

    template<class T>
    QWeakPointer<T> weakRef() {
        if(mThisWeak.isNull()) {
            if(mTmpPtr.isNull()) {
                mTmpPtr = QSharedPointer<SelfRef>(this);
            }
            return qSharedPointerCast<T>(mTmpPtr).toWeakRef();
        }
        return ref<T>().toWeakRef();
    }

private:
    QSharedPointer<SelfRef> mTmpPtr;
    QWeakPointer<SelfRef> mThisWeak;
};

#include <memory>

class StdSelfRef {
public:
    virtual ~StdSelfRef() {}
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
