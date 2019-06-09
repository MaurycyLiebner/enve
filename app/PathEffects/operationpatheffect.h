#ifndef OPERATIONPATHEFFECT_H
#define OPERATIONPATHEFFECT_H
#include "PathEffects/patheffect.h"
class ComboBoxProperty;
class BoxTargetProperty;

class OperationPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    OperationPathEffect();
public:
    void apply(const qreal relFrame,
               const SkPath &src,
               SkPath * const dst);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice * const src);

    bool hasReasonsNotToApplyUglyTransform() {
        return true;//mBoxTarget->getTarget() != nullptr;
    }
private:
    qsptr<ComboBoxProperty> mOperationType;
    qsptr<BoxTargetProperty> mBoxTarget;
};

#endif // OPERATIONPATHEFFECT_H
