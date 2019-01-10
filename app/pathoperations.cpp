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
    SkStroke strokerSk;
    strokerSk.setJoin(SkPaint::kRound_Join);
    strokerSk.setCap(SkPaint::kRound_Cap);
    strokerSk.setWidth(static_cast<SkScalar>(qAbs(widthT*2.)));
    SkPath outline;
    strokerSk.strokePath(src, &outline);
    SkPath extOutlineOnly;
    SkPath::Iter iter(outline, false);

    int i = 0;
    bool isOuter = false;
    auto srcCubicList = gPathToQCubicSegs2D(src);
    bool wantOuter = ((widthT > 0) == gCubicListClockWise(srcCubicList));
    for(;;) {
        SkPoint  pts[4];
        switch(iter.next(pts, true, true)) {
            case SkPath::kLine_Verb:
                if(isOuter == wantOuter) {
                    extOutlineOnly.lineTo(pts[1]);
                }
                break;
            case SkPath::kQuad_Verb:
                if(isOuter == wantOuter) {
                    extOutlineOnly.quadTo(pts[1], pts[2]);
                }
                break;
            case SkPath::kConic_Verb:
                if(isOuter == wantOuter) {
                    extOutlineOnly.conicTo(pts[1], pts[2], iter.conicWeight());
                }
                break;
            case SkPath::kCubic_Verb:
                if(isOuter == wantOuter) {
                    extOutlineOnly.cubicTo(pts[1], pts[2], pts[3]);
                }
                break;
            case SkPath::kClose_Verb:
                if(isOuter == wantOuter) {
                    extOutlineOnly.close();
                }
                break;
            case SkPath::kMove_Verb:
                if(i % 2 == 0) {
                    isOuter = false;
                    i++;
                } else {
                    isOuter = true;
                    i++;
                }
                /*if(isOuter == wantOuter) */extOutlineOnly.moveTo(pts[0]);
                break;
            case SkPath::kDone_Verb:
                goto DONE;
        }
    }
DONE:
    auto cubicList = gPathToQCubicSegs2D(extOutlineOnly);
    cubicList = gCubicIntersectList(cubicList);
    cubicList = gRemoveAllPointsCloserThan(
                qMax(0., abs(widthT) - 1.), src, cubicList);
    if(widthT > 0) {
        cubicList = gRemoveAllPointsInsidePath(src, cubicList);
    } else {
        cubicList = gRemoveAllPointsOutsidePath(src, cubicList);
    }
    extOutlineOnly = gCubicListToSkPath(cubicList);

    *dst = extOutlineOnly;
}
