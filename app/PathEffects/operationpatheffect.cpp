#include "operationpatheffect.h"
#include "pathoperations.h"
#include "Boxes/pathbox.h"

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

void OperationPathEffect::filterPathForRelFrame(const int &relFrame,
                                          const SkPath &src,
                                          SkPath *dst,
                                          const qreal &,
                                          const bool &) {
    PathBox *pathBox = GetAsPtr(mBoxTarget->getTarget(), PathBox);
    QString operation = mOperationType->getCurrentValueName();
    gApplyOperation(relFrame, src, dst, pathBox,
             mParentPathBox, operation);
}

void OperationPathEffect::filterPathForRelFrameF(const qreal &relFrame,
                                           const SkPath &src,
                                           SkPath *dst,
                                           const bool &) {
    PathBox *pathBox = GetAsPtr(mBoxTarget->getTarget(), PathBox);
    QString operation = mOperationType->getCurrentValueName();
    gApplyOperationF(relFrame, src, dst, pathBox,
              mParentPathBox, operation);
}
