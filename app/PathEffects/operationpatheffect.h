#ifndef OPERATIONPATHEFFECT_H
#define OPERATIONPATHEFFECT_H
#include "patheffect.h"

class OperationPathEffect : public PathEffect {
    Q_OBJECT
    friend class SelfRef;
public:
    OperationPathEffect(PathBox *parentPath,
                        const bool &outlinePathEffect);


    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const qreal &,
                               const bool &);
    void filterPathForRelFrameF(const qreal &relFrame,
                                const SkPath &src,
                                SkPath *dst,
                                const bool &);

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
