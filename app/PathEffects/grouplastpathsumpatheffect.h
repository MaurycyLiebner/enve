#ifndef GROUPLASTPATHSUMPATHEFFECT_H
#define GROUPLASTPATHSUMPATHEFFECT_H
#include "patheffect.h"

class GroupLastPathSumPathEffect : public PathEffect {
    Q_OBJECT
public:
    GroupLastPathSumPathEffect(BoxesGroup *parentPath,
                               const bool &outlinePathEffect);


    void filterPathForRelFrame(const int &relFrame,
                               const SkPath &src,
                               SkPath *dst,
                               const qreal &,
                               const bool &groupPathSum);
    void filterPathForRelFrameF(const qreal &relFrame,
                                const SkPath &src,
                                SkPath *dst,
                                const bool &groupPathSum);
    void writeProperty(QIODevice *target) {
        Q_UNUSED(target);
    }
    void readProperty(QIODevice *target) {
        Q_UNUSED(target);
    }

//    bool hasReasonsNotToApplyUglyTransform() {
//        return true;//mBoxTarget->getTarget() != nullptr;
//    }

    void setParentGroup(BoxesGroup *parent);
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);
private:
    qptr<BoxesGroup> mParentGroup;
};
#endif // GROUPLASTPATHSUMPATHEFFECT_H
