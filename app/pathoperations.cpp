#include "pathoperations.h"
#include "Boxes/pathbox.h"
#include "Animators/transformanimator.h"
#include "exceptions.h"
#include "SkPathOps.h"
#include "pointhelpers.h"

void gApplyOperation(const int &relFrame, const SkPath &src,
                     SkPath *dst, PathBox *srcBox,
                     PathBox *dstBox, const QString &operation,
                     const bool &groupSum) {
    gApplyOperationF(relFrame, src, dst, srcBox,
                    dstBox, operation, groupSum);
}

void gApplyOperationF(const qreal &relFrame, const SkPath &src,
                      SkPath *dst, PathBox *srcBox,
                      PathBox *dstBox, const QString &operation,
                      const bool &groupSum) {
    if(srcBox == nullptr) {
        *dst = src;
        return;
    }
    qreal absFrame = dstBox->
            prp_relFrameToAbsFrameF(relFrame);
    qreal pathBoxRelFrame = srcBox->
            prp_absFrameToRelFrameF(absFrame);
    SkPath boxPath;
    if(groupSum) {
        boxPath = srcBox->getPathWithEffectsUntilGroupSumAtRelFrameF(pathBoxRelFrame);
    } else {
        boxPath = srcBox->getPathWithThisOnlyEffectsAtRelFrameF(pathBoxRelFrame);
    }
    if(src.isEmpty()) {
        *dst = boxPath;
        return;
    }
    if(boxPath.isEmpty()) {
        *dst = src;
        return;
    }
    SkPathOp op;
    // "Union" << "Difference" << "Intersection" << "Exclusion"
    if(operation == "Union") {
        op = SkPathOp::kUnion_SkPathOp;
    } else if(operation == "Difference") {
        op = SkPathOp::kDifference_SkPathOp;
    } else if(operation == "Intersection") {
        op = SkPathOp::kIntersect_SkPathOp;
    } else { // "Exclusion"
        op = SkPathOp::kXOR_SkPathOp;
    }
    QMatrix pathBoxMatrix =
            srcBox->getTransformAnimator()->
                getCombinedTransformMatrixAtRelFrameF(
                    pathBoxRelFrame);
    QMatrix parentBoxMatrix =
            dstBox->getTransformAnimator()->
                getCombinedTransformMatrixAtRelFrameF(
                    relFrame);
    boxPath.transform(
                QMatrixToSkMatrix(
                    pathBoxMatrix*parentBoxMatrix.inverted()));
    if(!Op(src, boxPath, op, dst)) {
        RuntimeThrow("Operation Failed.");
    }
}

void gSolidify(const qreal &widthT, const SkPath &src, SkPath *dst) {
    if(isZero4Dec(widthT)) {
        *dst = src;
        return;
    }
    SkPath outline;

    SkPathOp op = widthT < 0 ? SkPathOp::kDifference_SkPathOp :
                               SkPathOp::kUnion_SkPathOp;


    SkOpBuilder builder;
    builder.add(src, SkPathOp::kUnion_SkPathOp);

    qreal aWidth2 = qAbs(widthT*2);
    SkStroke strokerSk;
    strokerSk.setJoin(SkPaint::kRound_Join);
    strokerSk.setCap(SkPaint::kRound_Cap);
    strokerSk.setWidth(static_cast<SkScalar>(aWidth2));
    auto func = [&builder, &strokerSk, &op](const SkPath& seg) {
        SkPath outline;
        strokerSk.strokePath(seg, &outline);
        builder.add(outline, op);
    };
    gForEverySegmentInPath(src, func);

    builder.resolve(dst);
}
