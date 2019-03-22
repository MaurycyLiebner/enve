#ifndef LENGTHPATHEFFECT_H
#define LENGTHPATHEFFECT_H
#include "PathEffects/patheffect.h"

class LengthPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    LengthPathEffect(const bool &outlinePathEffect);
public:
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
