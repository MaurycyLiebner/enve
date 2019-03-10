#ifndef DUPLICATEPATHEFFECT_H
#define DUPLICATEPATHEFFECT_H
#include "patheffect.h"

class DuplicatePathEffect : public PathEffect {
    Q_OBJECT
    friend class SelfRef;
public:
    DuplicatePathEffect(const bool &outlinePathEffect);

    void filterPathForRelFrame(const qreal &relFrame,
                                const SkPath &src,
                                SkPath *dst,
                                const bool &);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:
    qsptr<QPointFAnimator> mTranslation;
};

#endif // DUPLICATEPATHEFFECT_H
