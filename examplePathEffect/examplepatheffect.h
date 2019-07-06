#ifndef EXAMPLEPATHEFFECT_H
#define EXAMPLEPATHEFFECT_H

#include "examplepatheffect_global.h"

class ExamplePathEffect000 : public CustomPathEffect {
public:
    ExamplePathEffect000();

    CustomIdentifier getIdentifier() const;

    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);
private:
    qsptr<QrealAnimator> mInfluence;
};

#endif // EXAMPLEPATHEFFECT_H
