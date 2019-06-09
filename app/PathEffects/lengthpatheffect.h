#ifndef LENGTHPATHEFFECT_H
#define LENGTHPATHEFFECT_H
#include "PathEffects/patheffect.h"
class __attribute__ ((deprecated("Use SubPathEffect instead")))
      LengthPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    LengthPathEffect();
public:
    void apply(const qreal relFrame, const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice * const src);
private:
    qsptr<QrealAnimator> mLength;
    qsptr<BoolProperty> mReverse;
};

#endif // LENGTHPATHEFFECT_H
