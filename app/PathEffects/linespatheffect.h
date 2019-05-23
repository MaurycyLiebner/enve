#ifndef LINESPATHEFFECT_H
#define LINESPATHEFFECT_H
#include "PathEffects/patheffect.h"

class LinesPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    LinesPathEffect(const bool &outlinePathEffect);
public:
    void apply(const qreal &relFrame,
               const SkPath &src,
               SkPath * const dst);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:
    qsptr<QrealAnimator> mAngle;
    qsptr<QrealAnimator> mDistance;
};

#endif // LINESPATHEFFECT_H
