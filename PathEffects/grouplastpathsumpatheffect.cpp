#include "grouplastpathsumpatheffect.h"
#include "Boxes/boxesgroup.h"
#include "pathoperations.h"

GroupLastPathSumPathEffect::GroupLastPathSumPathEffect(
        BoxesGroup *parentGroup,
        const bool &outlinePathEffect) :
    PathEffect("sum path effect", GROUP_SUM_PATH_EFFECT,
               outlinePathEffect) {
    mParentGroup = parentGroup;
}

void GroupLastPathSumPathEffect::filterPathForRelFrame(
        const int &relFrame, const SkPath &src, SkPath *dst,
        const qreal &, const bool &groupPathSum) {
    if(!groupPathSum) {
        *dst = src;
        return;
    }
    QString operation = "Union";
    const QList<BoundingBoxQSPtr> &boxList =
            mParentGroup->getContainedBoxesList();
    QList<PathBox*> pathBoxes;
    foreach(const QSharedPointer<BoundingBox> &pathBox, boxList) {
        if(pathBox->SWT_isPathBox()) {
            pathBoxes << GetAsPtr(pathBox.data(), PathBox);
        }
    }
    if(pathBoxes.count() < 2) {
        *dst = src;
        return;
    }
    PathBox *lastPath = pathBoxes.takeLast();
    SkPath srcT = src;
    foreach(PathBox *pathBox, pathBoxes) {
        applyOperation(relFrame, srcT, dst, pathBox,
                 lastPath, operation, true);
        srcT = *dst;
    }
}

void GroupLastPathSumPathEffect::filterPathForRelFrameF(const qreal &relFrame,
                                                       const SkPath &src,
                                                       SkPath *dst,
                                                       const bool &groupPathSum) {
    if(!groupPathSum) {
        *dst = src;
        return;
    }
    QString operation = "Union";
    const QList<QSharedPointer<BoundingBox> > &boxList =
            mParentGroup->getContainedBoxesList();
    QList<PathBox*> pathBoxes;
    foreach(const QSharedPointer<BoundingBox> &pathBox, boxList) {
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
    foreach(PathBox *pathBox, pathBoxes) {
        applyOperationF(relFrame, srcT, dst, pathBox,
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
                        const bool &parentMainTarget) {
    return Animator::SWT_shouldBeVisible(
                rules,
                parentSatisfies,
                parentMainTarget);
}
