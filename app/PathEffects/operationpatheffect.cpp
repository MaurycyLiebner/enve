#include "operationpatheffect.h"
#include "Properties/boxtargetproperty.h"
#include "Properties/comboboxproperty.h"
#include "Boxes/pathbox.h"
#include "pathoperations.h"

OperationPathEffect::OperationPathEffect(PathBox *parentPath,
                             const bool &outlinePathEffect) :
    PathEffect("path operation effect",
               SUM_PATH_EFFECT, outlinePathEffect) {
    mOperationType = SPtrCreate(ComboBoxProperty)(
                "operation type",
                 QStringList() << "Union" <<
                 "Difference" << "Intersection" <<
                 "Exclusion");
    mBoxTarget = SPtrCreate(BoxTargetProperty)("sum with");
    mParentPathBox = parentPath;

    ca_addChildAnimator(mBoxTarget);
    ca_addChildAnimator(mOperationType);
}

void OperationPathEffect::apply(const qreal &relFrame,
                                const SkPath &src,
                                SkPath * const dst) {
    const auto pathBox = GetAsPtr(mBoxTarget->getTarget(), PathBox);
    QString operation = mOperationType->getCurrentValueName();
    gApplyOperation(relFrame, src, dst, pathBox,
                     mParentPathBox, operation);
}
