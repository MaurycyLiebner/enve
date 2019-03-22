#ifndef DUPLICATEPATHEFFECT_H
#define DUPLICATEPATHEFFECT_H
#include "PathEffects/patheffect.h"
class QPointFAnimator;

class DuplicatePathEffect : public PathEffect {
    Q_OBJECT
    friend class SelfRef;
public:
    DuplicatePathEffect(const bool &outlinePathEffect);

    void apply(const qreal &relFrame,
               const SkPath &src,
               SkPath * const dst);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
private:
    qsptr<QPointFAnimator> mTranslation;
};

#endif // DUPLICATEPATHEFFECT_H
