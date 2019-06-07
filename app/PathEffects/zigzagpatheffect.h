#ifndef ZIGZAGPATHEFFECT_H
#define ZIGZAGPATHEFFECT_H
#include "PathEffects/patheffect.h"

class ZigZagPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    ZigZagPathEffect(const bool outlinePathEffect);
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

#endif // ZIGZAGPATHEFFECT_H
