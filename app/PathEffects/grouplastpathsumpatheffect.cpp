#include "grouplastpathsumpatheffect.h"
#include "Boxes/boxesgroup.h"
#include "pathoperations.h"
#include "Boxes/pathbox.h"

GroupLastPathSumPathEffect::GroupLastPathSumPathEffect(
        BoxesGroup *parentGroup,
        const bool &outlinePathEffect) :
    PathEffect("sum path effect", GROUP_SUM_PATH_EFFECT,
               outlinePathEffect) {
    mParentGroup = parentGroup;
}

void GroupLastPathSumPathEffect::filterPathForRelFrame(const qreal &relFrame,
                                                       const SkPath &src,
                                                       SkPath *dst,
                                                       const bool &groupPathSum) {
    if(!groupPathSum) {
        *dst = src;
        return;
    }
    QString operation = "Union";
    const QList<qsptr<BoundingBox> > &boxList =
            mParentGroup->getContainedBoxesList();
    QList<PathBox*> pathBoxes;
    for(const auto& pathBox : boxList) {
        if(pathBox->SWT_isPathBox()) {
            pathBoxes << GetAsPtr(pathBox, PathBox);
        }
    }
    if(pathBoxes.count() < 2) {
        *dst = src;
        return;
    }
    PathBox *lastPath = pathBoxes.takeLast();
    SkPath srcT = src;
    for(PathBox *pathBox : pathBoxes) {
        gApplyOperationF(relFrame, srcT, dst, pathBox,
                        lastPath, operation, true);
        srcT = *dst;
    }
}


void GroupLastPathSumPathEffect::setParentGroup(BoxesGroup *parent) {
    mParentGroup = parent;
}

bool GroupLastPathSumPathEffect::SWT_shouldBeVisible(
                        const SWT_RulesCollection &rules,
                        const bool &parentSatisfies,
                        const bool &parentMainTarget) const {
    return Animator::SWT_shouldBeVisible(
                rules,
                parentSatisfies,
                parentMainTarget);
}
