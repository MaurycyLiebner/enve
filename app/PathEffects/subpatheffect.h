#ifndef SUBPATHEFFECT_H
#define SUBPATHEFFECT_H
#include "PathEffects/patheffect.h"
class SubPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    SubPathEffect(const bool outlinePathEffect);
public:
    void readLengthEffect(QIODevice * const target)
    __attribute__ ((deprecated("Use only to support old saves")));

    void apply(const qreal relFrame, const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
private:
    qsptr<QrealAnimator> mMin;
    qsptr<QrealAnimator> mMax;
};

#endif // SUBPATHEFFECT_H
