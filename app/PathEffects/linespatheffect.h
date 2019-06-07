#ifndef LINESPATHEFFECT_H
#define LINESPATHEFFECT_H
#include "PathEffects/patheffect.h"

class LinesPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    LinesPathEffect(const bool outlinePathEffect);
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);

    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
private:
    qsptr<QrealAnimator> mAngle;
    qsptr<QrealAnimator> mDistance;
};

#endif // LINESPATHEFFECT_H
