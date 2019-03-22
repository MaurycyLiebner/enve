#ifndef OPERATIONPATHEFFECT_H
#define OPERATIONPATHEFFECT_H
#include "PathEffects/patheffect.h"
class ComboBoxProperty;
class BoxTargetProperty;

class OperationPathEffect : public PathEffect {
    friend class SelfRef;
protected:
    OperationPathEffect(PathBox * const parentPath,
                        const bool &outlinePathEffect);
public:
    void apply(const qreal &relFrame,
               const SkPath &src,
               SkPath * const dst);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    bool hasReasonsNotToApplyUglyTransform() {
        return true;//mBoxTarget->getTarget() != nullptr;
    }
private:
    qptr<PathBox> mParentPathBox;
    qsptr<ComboBoxProperty> mOperationType;
    qsptr<BoxTargetProperty> mBoxTarget;
};

#endif // OPERATIONPATHEFFECT_H
