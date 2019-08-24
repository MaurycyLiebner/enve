// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "pathoperations.h"
#include "Boxes/pathbox.h"
#include "Animators/transformanimator.h"
#include "exceptions.h"
#include "pointhelpers.h"

void gApplyOperation(const qreal relFrame,
                     const SkPath &src,
                     SkPath * const dst,
                     PathBox * const srcBox,
                     PathBox * const dstBox,
                     const QString &operation) {
    if(!srcBox) {
        *dst = src;
        return;
    }
    const qreal absFrame = dstBox->prp_relFrameToAbsFrameF(relFrame);
    const qreal pathBoxRelFrame = srcBox->prp_absFrameToRelFrameF(absFrame);
    SkPath boxPath;
    boxPath = srcBox->getPathWithThisOnlyEffectsAtRelFrameF(pathBoxRelFrame);
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
    const auto srcBoxTrans = srcBox->getTransformAnimator();
    const QMatrix pathBoxMatrix = srcBoxTrans->
            getTotalTransformAtFrame(pathBoxRelFrame);
    const auto dstBoxTrans = dstBox->getTransformAnimator();
    const QMatrix parentBoxMatrix = dstBoxTrans->
            getTotalTransformAtFrame(relFrame);
    boxPath.transform(toSkMatrix(pathBoxMatrix*parentBoxMatrix.inverted()));
    if(!Op(src, boxPath, op, dst)) RuntimeThrow("Operation Failed.");
}

void gSolidify(const qreal widthT,
               const SkPath &src,
               SkPath * const dst) {
    if(isZero4Dec(widthT)) {
        *dst = src;
        return;
    }
    SkPathOp op = widthT < 0 ? SkPathOp::kDifference_SkPathOp :
                               SkPathOp::kUnion_SkPathOp;
    const qreal aWidth2 = qAbs(widthT*2);
    SkStroke strokerSk;
    strokerSk.setJoin(SkPaint::kRound_Join);
    strokerSk.setCap(SkPaint::kRound_Cap);
    strokerSk.setWidth(static_cast<float>(aWidth2));

    SkPath src2 = gPathToPolyline(src);
    SkPath outline;
    strokerSk.strokePath(src2, &outline);
    SkPath result;
    Op(src2, outline, op, dst);

//    SkOpBuilder builder;
//    builder.add(src, SkPathOp::kUnion_SkPathOp);
//    const auto func = [&builder, &strokerSk, &op](const qCubicSegment2D& seg) {
//        qCubicSegment2D segC = seg;
//        SkPath lineSeg;
//        lineSeg.moveTo(toSkPoint(seg.p0()));
//        if(!seg.isLine()) {
//            for(qreal len = 10; len < segC.length(); len += 10) {
//                lineSeg.lineTo(toSkPoint(segC.posAtLength(len)));
//            }
//        }
//        lineSeg.lineTo(toSkPoint(seg.p3()));
//        SkPath outline;
//        strokerSk.strokePath(lineSeg, &outline);
//        builder.add(outline, op);
//    };
////    const auto func = [&builder, &strokerSk, &op](const SkPath& seg) {
////        SkPath outline;
////        strokerSk.strokePath(seg, &outline);
////        builder.add(outline, op);
////    };
//    gForEverySegmentInPath(src, func);

//    builder.resolve(dst);
}
