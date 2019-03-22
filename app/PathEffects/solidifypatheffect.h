#ifndef SOLIDIFYPATHEFFECT_H
#define SOLIDIFYPATHEFFECT_H
#include "patheffect.h"

class SolidifyPathEffect : public PathEffect {
    Q_OBJECT
    friend class SelfRef;
public:
    SolidifyPathEffect(const bool &outlinePathEffect);

    void apply(const qreal &relFrame,
               const SkPath &src,
               SkPath * const dst);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:
    qsptr<QrealAnimator> mDisplacement;
};

#endif // SOLIDIFYPATHEFFECT_H
