#include "solidifypatheffect.h"
#include "pathoperations.h"

SolidifyPathEffect::SolidifyPathEffect(const bool &outlinePathEffect) :
    PathEffect("solidify effect", SOLIDIFY_PATH_EFFECT, outlinePathEffect) {
    mDisplacement = SPtrCreate(QrealAnimator)("displacement");
    mDisplacement->qra_setValueRange(0., 999.999);
    mDisplacement->qra_setCurrentValue(10.);

    ca_addChildAnimator(mDisplacement);
}

void SolidifyPathEffect::filterPathForRelFrame(const int &relFrame,
                                               const SkPath &src,
                                               SkPath *dst,
                                               const qreal &scale,
                                               const bool &) {
    qreal widthT = mDisplacement->getCurrentEffectiveValueAtRelFrame(relFrame)/scale;
    solidify(widthT, src, dst);
}

void SolidifyPathEffect::filterPathForRelFrameF(const qreal &relFrame,
                                                const SkPath &src,
                                                SkPath *dst,
                                                const bool &) {
    qreal widthT = mDisplacement->getCurrentEffectiveValueAtRelFrameF(relFrame);
    solidify(widthT, src, dst);
}

void SolidifyPathEffect::solidify(const qreal &widthT,
                                  const SkPath &src,
                                  SkPath *dst) {
    if(widthT < 0.001) {
        *dst = src;
        return;
    }
    SkStroke strokerSk;
    strokerSk.setWidth(widthT);
    SkPath outline;
    strokerSk.strokePath(src, &outline);
    SkPath extOutlineOnly;
    SkPath::Iter iter(outline, false);

    int i = 0;
    bool isOuter = false;
    for(;;) {
        SkPoint  pts[4];
        switch (iter.next(pts, false)) {
            case SkPath::kLine_Verb:
                if(isOuter) {
                    extOutlineOnly.lineTo(pts[1]);
                }
                break;
            case SkPath::kQuad_Verb:
                if(isOuter) {
                    extOutlineOnly.quadTo(pts[1], pts[2]);
                }
                break;
            case SkPath::kConic_Verb: {
                if(isOuter) {
                    extOutlineOnly.conicTo(pts[1], pts[2], iter.conicWeight());
                }
                break;
            } break;
            case SkPath::kCubic_Verb:
                if(isOuter) {
                    extOutlineOnly.cubicTo(pts[1], pts[2], pts[3]);
                }
                break;
            case SkPath::kClose_Verb:
                if(isOuter) {
                    extOutlineOnly.close();
                }
                break;
            case SkPath::kMove_Verb:
                if(i % 2 == 0) {
                    i++;
                    isOuter = false;
                } else {
                    isOuter = true;
                    i++;
                    extOutlineOnly.moveTo(pts[0]);
                }
                break;
            case SkPath::kDone_Verb:
                goto DONE;
        }
    }
DONE:
    *dst = extOutlineOnly;
    return;
//    FullVectorPath addToPath;
//    addToPath.generateFromPath(outline);
//    FullVectorPath addedPath;
//    addedPath.generateFromPath(src);

//    addToPath.intersectWith(&addedPath,
//                            true,
//                            true);
//    FullVectorPath targetPath;
//    targetPath.getSeparatePathsFromOther(&addToPath);
//    targetPath.getSeparatePathsFromOther(&addedPath);
//    targetPath.generateSinglePathPaths();

//    *dst = QPainterPathToSkPath(targetPath.getPath());
}
