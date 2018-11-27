#ifndef LENGTHPATHEFFECT_H
#define LENGTHPATHEFFECT_H
#include "patheffect.h"

class LengthPathEffect : public PathEffect {
    Q_OBJECT
public:
    LengthPathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const qreal &scale = 1.,
                               const bool & = false);
    void filterPathForRelFrameF(const qreal &relFrame,
                                const SkPath &src,
                                SkPath *dst,
                                const bool &);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
private:
    QrealAnimatorQSPtr mLength;
    BoolPropertyQSPtr mReverse;
};

#endif // LENGTHPATHEFFECT_H
