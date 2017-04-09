#ifndef SELFREF_H
#define SELFREF_H
#include <QSharedPointer>

class SelfRef {
public:
    template<class T>
    QSharedPointer<T> ref() {
        QSharedPointer<T> thisRef =
                qSharedPointerCast<T>(QSharedPointer<SelfRef>(this->mThisWeak));
        if(thisRef.isNull()) {
            thisRef = QSharedPointer<T>((T*)this);
            this->mThisWeak = qSharedPointerCast<SelfRef>(thisRef).toWeakRef();
        }
        return thisRef;
    }

private:
    QWeakPointer<SelfRef> mThisWeak;
};

#include <memory>

class StdSelfRef {
public:
    template<class T>
    std::shared_ptr<T> ref() {
        std::shared_ptr<T> thisRef =
                std::static_pointer_cast<T>(
                    std::shared_ptr<StdSelfRef>(this->mThisWeak));
        if(thisRef == nullptr) {
            thisRef = std::shared_ptr<T>((T*)this);
            this->mThisWeak = std::static_pointer_cast<StdSelfRef>(thisRef);
        }
        return thisRef;
    }

private:
    std::weak_ptr<StdSelfRef> mThisWeak;
};

#endif // SELFREF_H
