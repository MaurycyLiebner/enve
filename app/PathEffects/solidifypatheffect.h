#ifndef SOLIDIFYPATHEFFECT_H
#define SOLIDIFYPATHEFFECT_H
#include "PathEffects/patheffect.h"

class SolidifyPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    SolidifyPathEffect(const bool outlinePathEffect);
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice * const src);
private:
    qsptr<QrealAnimator> mDisplacement;
};

#endif // SOLIDIFYPATHEFFECT_H
