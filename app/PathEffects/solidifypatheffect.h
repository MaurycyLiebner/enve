#ifndef SOLIDIFYPATHEFFECT_H
#define SOLIDIFYPATHEFFECT_H
#include "patheffect.h"

class SolidifyPathEffect : public PathEffect {
    Q_OBJECT
    friend class SelfRef;
public:
    SolidifyPathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const qreal &scale = 1.,
                               const bool & = false);
    void filterPathForRelFrameF(const qreal &relFrame,
                                const SkPath &src,
                                SkPath *dst,
                                const bool &);
    void solidify(const qreal &widthT,
                  const SkPath &src,
                  SkPath *dst);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:
    qsptr<QrealAnimator> mDisplacement;
};

#endif // SOLIDIFYPATHEFFECT_H
