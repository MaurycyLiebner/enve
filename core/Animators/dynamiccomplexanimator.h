#ifndef DYNAMICCOMPLEXANIMATOR_H
#define DYNAMICCOMPLEXANIMATOR_H
#include "complexanimator.h"

template <class T>
qsptr<T> TCreateOnly(QIODevice * const src) {
    Q_UNUSED(src);
    return SPtrCreateTemplated(T)();
}

template <class T,
          void (T::*TWriteType)(QIODevice * const dst) const = nullptr,
          qsptr<T> (*TReadTypeAndCreate)(QIODevice * const src) = &TCreateOnly<T>>
class DynamicComplexAnimator : public ComplexAnimator {
    friend class SelfRef;
protected:
    DynamicComplexAnimator(const QString &name) :
        ComplexAnimator(name) {}
public:
    void writeProperty(QIODevice * const dst) const {
        const int nProps = ca_mChildAnimators.count();
        dst->write(rcConstChar(&nProps), sizeof(int));
        for(const auto& prop : ca_mChildAnimators) {
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
            ca_addChildAnimator(prop);
        }
    }

    void addChild(const qsptr<T>& child) {
        ca_addChildAnimator(child);
    }

    void removeChild(const qsptr<T>& child) {
        ca_removeChildAnimator(child);
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
    using ComplexAnimator::ca_addChildAnimator;
    using ComplexAnimator::ca_insertChildAnimator;
    using ComplexAnimator::ca_prependChildAnimator;

    using ComplexAnimator::ca_removeChildAnimator;
    using ComplexAnimator::ca_replaceChildAnimator;
    using ComplexAnimator::ca_takeChildAt;
};

#endif // DYNAMICCOMPLEXANIMATOR_H
