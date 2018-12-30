#include "displacepatheffect.h"
#include "pointhelpers.h"

DisplacePathEffect::DisplacePathEffect(const bool &outlinePathEffect) :
    PathEffect("displace effect", DISPLACE_PATH_EFFECT, outlinePathEffect) {
    mSegLength = SPtrCreate(QrealAnimator)("segment length");
    mMaxDev = SPtrCreate(QrealAnimator)("max deviation");
    mSmoothness = QrealAnimator::create0to1Animator("smoothness");
    mRandomize = SPtrCreate(BoolPropertyContainer)("randomize");
    mRandomizeStep = SPtrCreate(IntAnimator)("randomize step");
    mSmoothTransform = SPtrCreate(BoolPropertyContainer)("smooth progression");
    mEasing = QrealAnimator::create0to1Animator("ease in/out");
    mSeed = SPtrCreate(IntAnimator)("seed");
    mRepeat = SPtrCreate(BoolProperty)("repeat");

    mSeed->setIntValueRange(0, 9999);
    mSeed->setCurrentIntValue(qrand() % 9999);

    mSegLength->qra_setValueRange(1., 1000.);
    mSegLength->qra_setCurrentValue(20.);

    mMaxDev->qra_setValueRange(0., 1000.);
    mMaxDev->qra_setCurrentValue(20.);

    mRandomizeStep->setIntValueRange(1, 99);

    mRepeat->setValue(false);

    ca_addChildAnimator(mSeed);
    ca_addChildAnimator(mSegLength);
    ca_addChildAnimator(mMaxDev);
    ca_addChildAnimator(mSmoothness);
    ca_addChildAnimator(mRandomize);

    mRandomize->ca_addChildAnimator(mRandomizeStep);
    mRandomize->ca_addChildAnimator(mSmoothTransform);
    mSmoothTransform->ca_addChildAnimator(mEasing);
    mSmoothTransform->setValue(false);
    mRandomize->ca_addChildAnimator(mRepeat);
    mRandomize->setValue(false);
}

static void Perterb(SkPoint* p,
                    const SkVector& tangent,
                    SkScalar scale) {
    SkVector normal = tangent;
    SkPointPriv::RotateCCW(&normal);
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
    return static_cast<float>(qRandF(-1., 1.));
}

bool displaceFilterPath(SkPath* dst, const SkPath& src,
                        const SkScalar &maxDev,
                        const SkScalar &segLen,
                        const SkScalar &smoothness,
                        const uint32_t &seedAssist) {
    if(segLen < 0.01f) return false;
    dst->reset();
    SkPathMeasure meas(src, false);

    SkScalar scale = maxDev;
    SkPoint p;
    SkVector v;

    uint32_t seedContourInc = 0;;
    if(smoothness < 0.001f) {
        do {
            qsrand(seedAssist + seedContourInc);
            seedContourInc += 100;
            SkScalar length = meas.getLength();
            if(segLen * 2 > length) {
                meas.getSegment(0, length, dst, true);  // to short for us to mangle
                continue;
            }
            int nTot = SkScalarCeilToInt(length / segLen);
            int n = nTot;
            SkScalar distance = 0.f;
            SkScalar remLen = segLen*nTot - length;
            SkPoint firstP;
            if(meas.isClosed()) {
                n--;
                distance += (length + segLen)*0.5f;
            }

            if(meas.getPosTan(distance, &p, &v)) {
                //Perterb(&p, v, randFloat() * scale);
                dst->moveTo(p);
                firstP = p;
            }
            while(--n >= 0) {
                distance += segLen;
                if(meas.getPosTan(distance, &p, &v)) {
                    if(n == 0) {
                        SkScalar scaleT = 1.f - remLen/segLen;
                        Perterb(&p, v, randFloat() * scale * scaleT);

                    } else {
                        Perterb(&p, v, randFloat() * scale);
                    }
                    dst->lineTo(p);
                }
                if(distance + segLen > length) break;
            }
            if(meas.isClosed()) {
                distance = distance + segLen - length;
                while(--n >= 0) {
                    if(meas.getPosTan(distance, &p, &v)) {
                        if(n == 0) {
                            SkScalar scaleT = 1.f - remLen/segLen;
                            Perterb(&p, v, randFloat() * scale * scaleT);
                        } else {
                            Perterb(&p, v, randFloat() * scale);
                        }
                        dst->lineTo(p);
                    }
                    distance += segLen;
                }
                dst->close();
            }
        } while(meas.nextContour());
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
            qsrand(seedAssist + seedContourInc);
            seedContourInc += 100;
            SkScalar length = meas.getLength();
            if(segLen * 2 > length) {
                meas.getSegment(0, length, dst, true);  // to short for us to mangle
                continue;
            }
            int nTot = SkScalarCeilToInt(length / segLen);
            int n = nTot;
            SkScalar distance = 0.f;
            SkScalar remLen = segLen*nTot - length;
            SkScalar smoothLen = smoothness * segLen * 0.5f;

            if(meas.isClosed()) {
                n--;
                distance += (length + segLen)*0.5f;
            }

            if(meas.getPosTan(distance, &firstP, &v)) {
                //Perterb(&firstP, v, randFloat() * scale);
                lastP = firstP;
            }

            if(meas.isClosed()) {
                distance += segLen;
                if(meas.getPosTan(distance, &currP, &v)) {
                    Perterb(&currP, v, randFloat() * scale);
                    n--;
                    secondP = currP;
                }
                distance += segLen;
                if(meas.getPosTan(distance, &nextP, &v)) {
                    Perterb(&nextP, v, randFloat() * scale);
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
                distance += segLen;
                if(meas.getPosTan(distance, &nextP, &v)) {
                    if(n == 0) {
                        SkScalar scaleT = 1.f - remLen/segLen;
                        Perterb(&nextP, v, randFloat() * scale * scaleT);

                    } else {
                        Perterb(&nextP, v, randFloat() * scale);
                    }
                    getC1AndC2(lastP, currP, nextP,
                               &c1, &c2, smoothLen);


                    dst->cubicTo(lastC1, c2, currP);
                    lastC1 = c1;

                    lastP = currP;
                    currP = nextP;
                }
                if(distance + segLen > length) break;
            }

//            nextP = firstP;
//            getC1AndC2(lastP, currP, nextP,
//                       &c1, &c2, smoothLen);
//            dst->cubicTo(lastC1, c2, currP);
            if(meas.isClosed()) {
                distance = distance + segLen - length;
                while(--n >= 0) {
                    if(meas.getPosTan(distance, &nextP, &v)) {
                        if(n == 0) {
                            SkScalar scaleT = 1.f - remLen/segLen;
                            Perterb(&nextP, v, randFloat() * scale * scaleT);
                        } else {
                            Perterb(&nextP, v, randFloat() * scale);
                        }
                        getC1AndC2(lastP, currP, nextP,
                                   &c1, &c2, smoothLen);


                        dst->cubicTo(lastC1, c2, currP);
                        lastC1 = c1;

                        lastP = currP;
                        currP = nextP;
                    }
                    distance += segLen;
                }
                lastC1 = c1;

//                lastP = currP;
//                currP = nextP;
                nextP = firstP;

                getC1AndC2(lastP, currP, nextP,
                           &c1, &c2, smoothLen);

                dst->cubicTo(lastC1, c2, currP);
                lastC1 = c1;

                lastP = currP;
                currP = nextP;
                nextP = secondP;
                getC1AndC2(lastP, currP, nextP,
                           &c1, &c2, smoothLen);
                dst->cubicTo(lastC1, c2, currP);

                dst->close();
            }
        } while(meas.nextContour());
    }
    return true;
}

void DisplacePathEffect::filterPathForRelFrame(const int &relFrame,
                                               const SkPath &src,
                                               SkPath *dst,
                                               const qreal &scale,
                                               const bool &) {
    dst->reset();
    qsrand(static_cast<uint>(mSeed->getCurrentIntValue()));
    mSeedAssist = qrand() % 999999;
    int randStep = mRandomizeStep->getCurrentIntValueAtRelFrame(relFrame);
    uint32_t nextSeed;
    if(mRepeat->getValue()) {
        if(qAbs(relFrame / randStep) % 2 == 1) {
            nextSeed = mSeedAssist;
            mSeedAssist++;
        } else {
            nextSeed = mSeedAssist + 1;
        }
    } else if(mRandomize->getValue()) {
        mSeedAssist += qAbs(relFrame / randStep);
        nextSeed = mSeedAssist - 1;
    }
    if(mSmoothTransform->getValue() && mRandomize->getValue()) {
        SkPath path1;
        displaceFilterPath(&path1, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame)/scale,
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame)/scale,
                           mSmoothness->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSeedAssist);
        SkPath path2;
        qsrand(mSeed->getCurrentIntValue());
        displaceFilterPath(&path2, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame + randStep)/scale,
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame + randStep)/scale,
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
                           mMaxDev->qra_getEffectiveValueAtRelFrame(relFrame)/scale,
                           mSegLength->qra_getEffectiveValueAtRelFrame(relFrame)/scale,
                           mSmoothness->qra_getEffectiveValueAtRelFrame(relFrame),
                           mSeedAssist);
    }
}

void DisplacePathEffect::filterPathForRelFrameF(const qreal &relFrame,
                                                const SkPath &src,
                                                SkPath *dst, const bool &) {
    dst->reset();
    qsrand(mSeed->getCurrentIntValue());
    mSeedAssist = qrand() % 999999;
    int randStep = mRandomizeStep->getCurrentIntValueAtRelFrameF(relFrame);
    uint32_t nextSeed;
    if(mRepeat->getValue()) {
        if((qFloor(relFrame / randStep)) % 2 == 1) {
            nextSeed = mSeedAssist;
            mSeedAssist++;
        } else {
            nextSeed = mSeedAssist + 1;
        }
    } else if(mRandomize->getValue()) {
        mSeedAssist += qFloor(relFrame / randStep);
        nextSeed = mSeedAssist - 1;
    }
    if(mSmoothTransform->getValue() && mRandomize->getValue()) {
        SkPath path1;
        displaceFilterPath(&path1, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSegLength->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSmoothness->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSeedAssist);
        SkPath path2;
        qsrand(mSeed->getCurrentIntValue());
        displaceFilterPath(&path2, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrameF(relFrame + randStep),
                           mSegLength->qra_getEffectiveValueAtRelFrameF(relFrame + randStep),
                           mSmoothness->qra_getEffectiveValueAtRelFrameF(relFrame + randStep),
                           nextSeed);
        qreal weight = qAbs(qFloor(relFrame) % randStep)*1./randStep;
        qreal easing = mEasing->getCurrentEffectiveValueAtRelFrameF(relFrame);
        if(easing > 0.0001) {
            qreal tT = getBezierTValueForX(0., easing, 1. - easing, 1., weight);
            weight = calcCubicBezierVal(0., 0., 1., 1., tT);
        }
        path1.interpolate(path2, weight, dst);
    } else {
        displaceFilterPath(dst, src,
                           mMaxDev->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSegLength->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSmoothness->qra_getEffectiveValueAtRelFrameF(relFrame),
                           mSeedAssist);
    }
}
