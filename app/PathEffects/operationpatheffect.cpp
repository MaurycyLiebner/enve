#include "operationpatheffect.h"
#include "Properties/boxtargetproperty.h"
#include "Properties/comboboxproperty.h"
#include "Boxes/pathbox.h"
#include "pathoperations.h"

OperationPathEffect::OperationPathEffect() :
    PathEffect("path operation effect",
               OPERATION_PATH_EFFECT) {
    mOperationType = SPtrCreate(ComboBoxProperty)(
                "operation type",
                 QStringList() << "Union" <<
                 "Difference" << "Intersection" <<
                 "Exclusion");
    mBoxTarget = SPtrCreate(BoxTargetProperty)("target");

    ca_addChildAnimator(mBoxTarget);
    ca_addChildAnimator(mOperationType);
}

void OperationPathEffect::apply(const qreal relFrame,
                                const SkPath &src,
                                SkPath * const dst) {
    const auto pathBox = GetAsPtr(mBoxTarget->getTarget(), PathBox);
    const QString operation = mOperationType->getCurrentValueName();
    const auto idBox = [](Property * prop) {
        return prop->SWT_isBoundingBox();
    };
    const auto parentBox = getFirstAncestor<BoundingBox>(idBox);
    gApplyOperation(relFrame, src, dst, pathBox,
                    GetAsPtr(parentBox, PathBox), operation);
}
