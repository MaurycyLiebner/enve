#ifndef STATICCOMPLEXANIMATOR_H
#define STATICCOMPLEXANIMATOR_H
#include "complexanimator.h"

class StaticComplexAnimator : public ComplexAnimator {
public:
    StaticComplexAnimator(const QString &name);

    void writeProperty(QIODevice * const dst) const {
        for(const auto& prop : ca_mChildAnimators)
            prop->writeProperty(dst);
    }

    void readProperty(QIODevice * const src) {
        for(const auto& prop : ca_mChildAnimators)
            prop->readProperty(src);
    }

    using ComplexAnimator::ca_addChildAnimator;
    using ComplexAnimator::ca_insertChildAnimator;
    using ComplexAnimator::ca_prependChildAnimator;
};

#endif // STATICCOMPLEXANIMATOR_H
