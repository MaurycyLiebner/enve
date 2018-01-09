#include "patheffect.h"
#include "pointhelpers.h"
#include "pathoperations.h"
#include "skqtconversions.h"

PathEffect::PathEffect(const PathEffectType &type,
                       const bool &outlinePathEffect) {
    mPathEffectType = type;
    setIsOutlineEffect(outlinePathEffect);
}

DisplacePathEffect::DisplacePathEffect(const bool &outlinePathEffect) :
    PathEffect(DISPLACE_PATH_EFFECT, outlinePathEffect) {
    prp_setName("discrete effect");

    mSegLength->prp_setName("segment length");
    mSegLength->qra_setValueRange(0., 1000.);
    mSegLength->qra_setCurrentValue(20.);

    mMaxDev->prp_setName("max deviation");
    mMaxDev->qra_setValueRange(0., 1000.);
    mMaxDev->qra_setCurrentValue(20.);

    mSmoothness->prp_setName("smoothness");
    mSmoothness->qra_setValueRange(0., 1.);

    mEasing->prp_setName("ease in/out");
    mEasing->qra_setValueRange(0., 1.);

    mRandomize->prp_setName("randomize");

    mSeed->prp_setName("seed");
    mSeed->setIntValueRange(0, 9999);
    mSeed->setCurrentIntValue(qrand() % 9999, false);

    mRandomizeStep->prp_setName("rand frame step");
    mRandomizeStep->setIntValueRange(1, 99);

    mSmoothTransform->prp_setName("smooth progression");

    mRepeat->prp_setName("repeat");
    mRepeat->setValue(false);

    ca_addChildAnimator(mSegLength.data());
    ca_addChildAnimator(mMaxDev.data());
    ca_addChildAnimator(mSmoothness.data());
    ca_addChildAnimator(mRandomize.data());
    mRandomize->ca_addChildAnimator(mSeed.data());
    mRandomize->ca_addChildAnimator(mRandomizeStep.data());
    mRandomize->ca_addChildAnimator(mSmoothTransform.data());
    mSmoothTransform->ca_addChildAnimator(mEasing.data());
    mSmoothTransform->setValue(false);
    mRandomize->ca_addChildAnimator(mRepeat.data());
    mRandomize->setValue(false);
}

static void Perterb(SkPoint* p,
                    const SkVector& tangent,
                    SkScalar scale) {
    SkVector normal = tangent;
    normal.rotateCCW();
    normal.setLength(scale);
    *p += normal;
}

class LCGRandom {
public:
    LCGRandom(uint32_t seed) : fSeed(seed) {}

    /** Return the next pseudo random number expressed as a SkScalar
        in the range [-SK_Scalar1..SK_Scalar1).
    */
    SkScalar nextSScalar1() { return SkFixedToScalar(this->nextSFixed1()); }

private:
    /** Return the next pseudo random number as an unsigned 32bit value.
    */
    uint32_t nextU() { uint32_t r = fSeed * kMul + kAdd; fSeed = r; return r; }

    /** Return the next pseudo random number as a signed 32bit value.
     */
    int32_t nextS() { return (int32_t)this->nextU(); }

    /** Return the next pseudo random number expressed as a signed SkFixed
     in the range [-SK_Fixed1..SK_Fixed1).
     */
    SkFixed nextSFixed1() { return this->nextS() >> 15; }

    //  See "Numerical Recipes in C", 1992 page 284 for these constants
    enum {
        kMul = 1664525,
        kAdd = 1013904223
    };
    uint32_t fSeed;
};

void getC1AndC2(const SkPoint &lastP,
                const SkPoint &currP,
                const SkPoint &nextP,
                SkPoint *c1, SkPoint *c2,
                const SkScalar &smoothLen) {
    SkPoint sLastP = lastP - currP;
    sLastP.setLength(1.f);
    SkPoint sNextP = nextP - currP;
    sNextP.setLength(1.f);
    SkPoint vectP = (sLastP + sNextP)*0.5f;
    vectP.set(vectP.y(), -vectP.x());
    if(vectP.dot(lastP - currP) > 0) vectP.negate();

    SkScalar nextDist = (currP - nextP).length()*0.4f;
    if(smoothLen < nextDist) {
        vectP.setLength(smoothLen);
    } else {
        vectP.setLength(nextDist);
    }

    *c1 = currP + vectP;

    vectP.negate();
    SkScalar lastDist = (currP - lastP).length()*0.5f;
    if(smoothLen < lastDist) {
        vectP.setLength(smoothLen);
    } else {
        vectP.setLength(lastDist);
    }
    *c2 = currP + vectP;
}

float randFloat() {
    return (float)qRandF(-1., 1.);
}

bool displaceFilterPath(SkPath* dst, const SkPath& src,
                        const SkScalar &maxDev,
                        const SkScalar &segLen,
                        const SkScalar &smoothness,
                        const uint32_t &seedAssist) {
    if(segLen < 0.01) return false;
    dst->reset();
    SkPathMeasure meas(src, false);
    qsrand(seedAssist);

    /* Caller may supply their own seed assist, which by default is 0 */
    //uint32_t seed = seedAssist ^ SkScalarRoundToInt(meas.getLength());

    //LCGRandom rand(seed ^ ((seed << 16) | (seed >> 16)));
    SkScalar scale = maxDev;
    SkPoint p;
    SkVector v;
    if(smoothness < 0.001f) {
        do {
            SkPoint firstP;
            SkScalar length = meas.getLength();

            if(segLen * (2/* + doFill*/) > length) {
                meas.getSegment(0, length, dst, true);  // to short for us to mangle
            } else {
                int n = SkScalarRoundToInt(length / segLen);
                SkScalar delta = length / n;
                SkScalar distance = 0;

                if(meas.isClosed()) {
                    n--;
                    distance += delta/2;
                }

                if(meas.getPosTan(distance, &p, &v)) {
                    Perterb(&p, v, randFloat() /*rand.nextSScalar1()*/ * scale);
                    dst->moveTo(p);
                    firstP = p;
                }
                while(--n >= 0) {
                    distance += delta;
                    if(meas.getPosTan(distance, &p, &v)) {
                        Perterb(&p, v, randFloat() /*rand.nextSScalar1()*/ * scale);
                        dst->lineTo(p);
                    }
                }
                if(meas.isClosed()) {
                    //dst->lineTo(p);
                    dst->close();
                }
            }
        } while (meas.nextContour());
    } else {
        SkPoint firstP;
        SkPoint secondP;
        SkPoint thirdP;
        SkPoint lastP;
        SkPoint nextP;
        SkPoint currP;
        SkPoint lastC1;
        SkPoint c1;
        SkPoint c2;

        do {
            SkScalar smoothLen = smoothness * segLen * 0.5f;
            SkScalar length = meas.getLength();

            if(segLen * (2/* + doFill*/) > length) {
                meas.getSegment(0, length, dst, true);  // to short for us to mangle
            } else {
                int n = SkScalarRoundToInt(length / segLen);
                SkScalar delta = length / n;
                SkScalar distance = 0;

                if(meas.isClosed()) {
                    n--;
                    distance += delta/2;
                }

                if(meas.getPosTan(distance, &firstP, &v)) {
                    Perterb(&firstP, v, randFloat() /*rand.nextSScalar1()*/ * scale);
                    lastP = firstP;
                }

                if(meas.isClosed()) {
                    distance += delta;
                    if(meas.getPosTan(distance, &currP, &v)) {
                        Perterb(&currP, v, randFloat() /*rand.nextSScalar1()*/ * scale);
                        n--;
                        secondP = currP;
                    }
                    distance += delta;
                    if(meas.getPosTan(distance, &nextP, &v)) {
                        Perterb(&nextP, v, randFloat() /*rand.nextSScalar1()*/ * scale);
                        n--;
                        thirdP = nextP;

                        getC1AndC2(lastP, currP, nextP,
                                   &c1, &c2, smoothLen);

                        lastC1 = c1;

                        lastP = currP;
                        currP = nextP;
                    }
                } else {
                    currP = lastP;
                    lastC1 = currP;
                }
                dst->moveTo(lastP);
                while(--n >= 0) {
                    distance += delta;
                    if(meas.getPosTan(distance, &nextP, &v)) {
                        Perterb(&nextP, v, randFloat() /*rand.nextSScalar1()*/ * scale);
                        getC1AndC2(lastP, currP, nextP,
                                   &c1, &c2, smoothLen);


                        dst->cubicTo(lastC1, c2, currP);
                        lastC1 = c1;

                        lastP = currP;
                        currP = nextP;
                    }
                }

                nextP = firstP;
                getC1AndC2(lastP, currP, nextP,
                           &c1, &c2, smoothLen);
                dst->cubicTo(lastC1, c2, currP);
                if(meas.isClosed()) {
                    lastC1 = c1;

                    lastP = currP;
                    currP = nextP;
                    nextP = secondP;

                    getC1AndC2(lastP, currP, nextP,
                               &c1, &c2, smoothLen);

                    dst->cubicTo(lastC1, c2, currP);
                    lastC1 = c1;

                    lastP = currP;
                    currP = nextP;
                    nextP = thirdP;
                    getC1AndC2(lastP, currP, nextP,
                               &c1, &c2, smoothLen);
                    dst->cubicTo(lastC1, c2, currP);

                    dst->close();
                }
            }
        } while (meas.nextContour());
    }
    return true;
}

void DisplacePathEffect::filterPathForRelFrame(const int &relFrame,
                                               const SkPath &src,
                                               SkPath *dst, const bool &) {
    qsrand(mSeed->getCurrentIntValue());
    mSeedAssist = qrand() % 999999;
    int randStep = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame);
    uint32_t nextSeed;
    if(mRepeat->getValue()) {
        if((relFrame / randStep) % 2 == 1) {
            nextSeed = mSeedAssist;
            mSeedAssist++;
        } else {
            nextSeed = mSeedAssist + 1;
        }
    } else if(mRandomize->getValue()) {
        mSeedAssist += relFrame / randStep;
        nextSeed = mSeedAssist - 1;
    }
    if(mSmoothTransform->getValue()) {
        SkPath path1;
        displaceFilterPath(&path1, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSmoothness->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSeedAssist);
        SkPath path2;
        qsrand(mSeed->getCurrentIntValue());
        displaceFilterPath(&path2, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame + randStep),
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame + randStep),
                           mSmoothness->qra_getEffectiveValueAtRelFrame(relFrame + randStep),
                           nextSeed);
        qreal weight = qAbs(relFrame % randStep)*1./randStep;
        qreal easing = mEasing->getCurrentEffectiveValueAtRelFrame(relFrame);
        if(easing > 0.0001) {
            qreal tT = getBezierTValueForX(0., easing, 1. - easing, 1., weight);
            weight = calcCubicBezierVal(0., 0., 1., 1., tT);
        }
        path1.interpolate(path2, weight, dst);
    } else {
        displaceFilterPath(dst, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSmoothness->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSeedAssist);
    }
}

DuplicatePathEffect::DuplicatePathEffect(const bool &outlinePathEffect) :
    PathEffect(DUPLICATE_PATH_EFFECT, outlinePathEffect) {

    prp_setName("duplicate effect");

    mTranslation->prp_setName("translation");
    mTranslation->setCurrentPointValue(QPointF(10., 10.));

    ca_addChildAnimator(mTranslation.data());
}


void DuplicatePathEffect::filterPathForRelFrame(const int &relFrame,
                                                const SkPath &src,
                                                SkPath *dst,
                                                const bool &) {
    *dst = src;
    dst->addPath(src,
                 mTranslation->getEffectiveXValueAtRelFrame(relFrame),
                 mTranslation->getEffectiveYValueAtRelFrame(relFrame));
}


SolidifyPathEffect::SolidifyPathEffect(const bool &outlinePathEffect) :
    PathEffect(SOLIDIFY_PATH_EFFECT, outlinePathEffect) {

    prp_setName("solidify effect");

    mDisplacement->prp_setName("displacement");
    mDisplacement->qra_setValueRange(0., 999.999);
    mDisplacement->qra_setCurrentValue(10.);

    ca_addChildAnimator(mDisplacement.data());
}

void SolidifyPathEffect::filterPathForRelFrame(const int &relFrame,
                                               const SkPath &src,
                                               SkPath *dst,
                                               const bool &) {
    SkStroke strokerSk;
    qreal widthT = mDisplacement->getCurrentEffectiveValueAtRelFrame(relFrame);
    if(widthT < 0.001) {
        *dst = src;
        return;
    }
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
    FullVectorPath addToPath;
    addToPath.generateFromPath(outline);
    FullVectorPath addedPath;
    addedPath.generateFromPath(src);

    addToPath.intersectWith(&addedPath,
                            true,
                            true);
    FullVectorPath targetPath;
    targetPath.getSeparatePathsFromOther(&addToPath);
    targetPath.getSeparatePathsFromOther(&addedPath);
    targetPath.generateSinglePathPaths();

    *dst = QPainterPathToSkPath(targetPath.getPath());
}

SumPathEffect::SumPathEffect(PathBox *parentPath,
                             const bool &outlinePathEffect) :
    PathEffect(SUM_PATH_EFFECT, outlinePathEffect) {
    prp_setName("path operation effect");
    mParentPathBox = parentPath;
    ca_addChildAnimator(mBoxTarget.data());
    mOperationType->prp_setName("operation type");
    ca_addChildAnimator(mOperationType.data());
}

void sumPaths(const int &relFrame, const SkPath &src,
              SkPath *dst, PathBox *srcBox,
              PathBox *dstBox, const QString &operation,
              const bool &groupSum = false) {
    if(srcBox == NULL) {
        *dst = src;
        return;
    }
    int absFrame = dstBox->
            prp_relFrameToAbsFrame(relFrame);
    int pathBoxRelFrame = srcBox->
            prp_absFrameToRelFrame(absFrame);
    SkPath boxPath;
    if(groupSum) {
        boxPath = srcBox->getPathWithEffectsUntilGroupSumAtRelFrame(pathBoxRelFrame);
    } else {
        boxPath = srcBox->getPathWithThisOnlyEffectsAtRelFrame(pathBoxRelFrame);
    }
    if(src.isEmpty()) {
        *dst = boxPath;
        return;
    }
    if(boxPath.isEmpty()) {
        *dst = src;
        return;
    }
    bool unionInterThis, unionInterOther;
    // "Union" << "Difference" << "Intersection" << "Exclusion"
    if(operation == "Union") {
        unionInterOther = true;
        unionInterThis = true;
    } else if(operation == "Difference") {
        unionInterThis = false;
        unionInterOther = true;
    } else if(operation == "Intersection") {
        unionInterThis = false;
        unionInterOther = false;
    } else {
        unionInterThis = true;
        unionInterOther = false;
    }
    QMatrix pathBoxMatrix =
            srcBox->getTransformAnimator()->
                getCombinedTransformMatrixAtRelFrame(
                    pathBoxRelFrame);
    QMatrix parentBoxMatrix =
            dstBox->getTransformAnimator()->
                getCombinedTransformMatrixAtRelFrame(
                    relFrame);
    boxPath.transform(
                QMatrixToSkMatrix(
                    pathBoxMatrix*parentBoxMatrix.inverted()));
    FullVectorPath addToPath;
    addToPath.generateFromPath(boxPath);
    FullVectorPath addedPath;
    addedPath.generateFromPath(src);

    addToPath.intersectWith(&addedPath,
                            unionInterThis,
                            unionInterOther);
    FullVectorPath targetPath;
    targetPath.getSeparatePathsFromOther(&addToPath);
    targetPath.getSeparatePathsFromOther(&addedPath);
    targetPath.generateSinglePathPaths();

    *dst = QPainterPathToSkPath(targetPath.getPath());
    if(unionInterThis && !unionInterOther) {
        FullVectorPath addToPath2;
        addToPath2.generateFromPath(src);
        FullVectorPath addedPath2;
        addedPath2.generateFromPath(boxPath);

        addToPath2.intersectWith(&addedPath2,
                                 unionInterThis,
                                 unionInterOther);
        FullVectorPath targetPath2;
        targetPath2.getSeparatePathsFromOther(&addToPath2);
        targetPath2.getSeparatePathsFromOther(&addedPath2);
        targetPath2.generateSinglePathPaths();
        dst->addPath(QPainterPathToSkPath(targetPath2.getPath()));
    }
}


void SumPathEffect::filterPathForRelFrame(const int &relFrame,
                                          const SkPath &src,
                                          SkPath *dst,
                                          const bool &) {
    PathBox *pathBox = ((PathBox*)mBoxTarget->getTarget());
    QString operation = mOperationType->getCurrentValueName();
    sumPaths(relFrame, src, dst, pathBox,
             mParentPathBox, operation);
}

GroupLastPathSumPathEffect::GroupLastPathSumPathEffect(
        BoxesGroup *parentGroup,
        const bool &outlinePathEffect) :
    PathEffect(GROUP_SUM_PATH_EFFECT, outlinePathEffect) {
    prp_setName("sum path effect");
    mParentGroup = parentGroup;
}

void GroupLastPathSumPathEffect::filterPathForRelFrame(const int &relFrame,
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
            pathBoxes << (PathBox*)pathBox.data();
        }
    }
    if(pathBoxes.count() < 2) {
        *dst = src;
        return;
    }
    PathBox *lastPath = pathBoxes.takeLast();
    SkPath srcT = src;
    foreach(PathBox *pathBox, pathBoxes) {
        sumPaths(relFrame, srcT, dst, pathBox,
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
