#ifndef LENGTHPATHEFFECT_H
#define LENGTHPATHEFFECT_H
#include "patheffect.h"

class LengthPathEffect : public PathEffect {
    Q_OBJECT
public:
    LengthPathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrameF(const qreal &relFrame,
                                const SkPath &src,
                                SkPath *dst,
                                const bool &);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:
    qsptr<QrealAnimator> mLength;
    qsptr<BoolProperty> mReverse;
};

#endif // LENGTHPATHEFFECT_H
