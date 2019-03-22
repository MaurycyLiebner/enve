#ifndef LENGTHPATHEFFECT_H
#define LENGTHPATHEFFECT_H
#include "patheffect.h"

class LengthPathEffect : public PathEffect {
public:
    LengthPathEffect(const bool &outlinePathEffect);

    void apply(const qreal &relFrame,
               const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:
    qsptr<QrealAnimator> mLength;
    qsptr<BoolProperty> mReverse;
};

#endif // LENGTHPATHEFFECT_H
