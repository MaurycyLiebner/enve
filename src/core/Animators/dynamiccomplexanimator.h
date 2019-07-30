#ifndef DYNAMICCOMPLEXANIMATOR_H
#define DYNAMICCOMPLEXANIMATOR_H
#include "complexanimator.h"

template <class T>
class DynamicComplexAnimatorBase : public ComplexAnimator {
protected:
    DynamicComplexAnimatorBase(const QString &name) :
        ComplexAnimator(name) {}
public:
    void addChild(const qsptr<T>& child) {
        ca_addChild(child);
    }

    void removeChild(const qsptr<T>& child) {
        ca_removeChild(child);
    }

    qsptr<T> takeChildAt(const int id) {
        return GetAsSPtrTemplated(ca_takeChildAt(id), T);
    }

    void prependChild(T * const oldChild, const qsptr<T>& newChild) {
        ca_prependChildAnimator(oldChild, newChild);
    }

    void replaceChild(const qsptr<T>& oldChild,
                      const qsptr<T> &newChild) {
        ca_replaceChildAnimator(oldChild, newChild);
    }

    using ComplexAnimator::ca_removeAllChildAnimators;
private:
    using ComplexAnimator::ca_addChild;
    using ComplexAnimator::ca_insertChild;
    using ComplexAnimator::ca_prependChildAnimator;

    using ComplexAnimator::ca_removeChild;
    using ComplexAnimator::ca_replaceChildAnimator;
    using ComplexAnimator::ca_takeChildAt;
};

template <class T>
qsptr<T> TCreateOnly(QIODevice * const src) {
    Q_UNUSED(src);
    return SPtrCreateTemplated(T)();
}

template <class T,
          void (T::*TWriteType)(QIODevice * const dst) const = nullptr,
          qsptr<T> (*TReadTypeAndCreate)(QIODevice * const src) = &TCreateOnly<T>>
class DynamicComplexAnimator : public DynamicComplexAnimatorBase<T> {
    friend class SelfRef;
protected:
    DynamicComplexAnimator(const QString &name) :
        DynamicComplexAnimatorBase<T>(name) {}
public:
    void writeProperty(QIODevice * const dst) const {
        const int nProps = this->ca_mChildAnimators.count();
        dst->write(rcConstChar(&nProps), sizeof(int));
        for(const auto& prop : this->ca_mChildAnimators) {
            const auto TProp = static_cast<T*>(prop.get());
            if(TWriteType) (TProp->*TWriteType)(dst);
            TProp->writeProperty(dst);
        }
    }

    void readProperty(QIODevice * const src) {
        int nProps;
        src->read(rcChar(&nProps), sizeof(int));
        for(int i = 0; i < nProps; i++) {
            const auto prop = TReadTypeAndCreate(src);
            prop->readProperty(src);
            this->addChild(prop);
        }
    }
};

#endif // DYNAMICCOMPLEXANIMATOR_H
