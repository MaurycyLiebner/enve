#include "patheffect.h"

PathEffect::PathEffect(const PathEffectType &type) {
    mPathEffectType = type;
}

qreal PathEffect::getMargin() { return 0.; }

DisplacePathEffect::DisplacePathEffect() :
    PathEffect(DISCRETE_PATH_EFFECT) {
    prp_setName("discrete effect");

    mSegLength->prp_setName("segment length");
    mSegLength->qra_setValueRange(0., 1000.);

    mMaxDev->prp_setName("max deviation");
    mMaxDev->qra_setValueRange(0., 1000.);

    mSmoothness->prp_setName("smoothness");
    mSmoothness->qra_setValueRange(0., 1.);

    ca_addChildAnimator(mSegLength.data());
    ca_addChildAnimator(mMaxDev.data());
    ca_addChildAnimator(mSmoothness.data());
}

qreal DisplacePathEffect::getMargin() {
    return mMaxDev->qra_getCurrentValue();
}

Property *DisplacePathEffect::makeDuplicate() {
    DisplacePathEffect *newEffect = new DisplacePathEffect();
    makeDuplicate(newEffect);
    return newEffect;
}

void DisplacePathEffect::makeDuplicate(Property *target) {
    DisplacePathEffect *effectTarget = (DisplacePathEffect*)target;

    effectTarget->duplicateAnimatorsFrom(mSegLength.data(),
                                         mMaxDev.data());
}

void DisplacePathEffect::duplicateAnimatorsFrom(QrealAnimator *segLen,
                                                QrealAnimator *maxDev) {
    segLen->makeDuplicate(mSegLength.data());
    maxDev->makeDuplicate(mMaxDev.data());
}

static void Perterb(SkPoint* p,
                    const SkVector& tangent,
                    SkScalar scale) {
    SkVector normal = tangent;
    normal.rotateCCW();
    normal.setLength(scale);
    *p += normal;
}

static void makeTangent(SkPoint* p,
                        const SkVector& tangent,
                        SkScalar scale) {
    SkVector normal = tangent;
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

bool displaceFilterPath(SkPath* dst, const SkPath& src,
                        const SkScalar &maxDev,
                        const SkScalar &segLen,
                        const SkScalar &smoothness,
                        const uint32_t &seedAssist) {
    if(segLen < 0.01) return false;
    dst->reset();
    SkPathMeasure meas(src, false);

    /* Caller may supply their own seed assist, which by default is 0 */
    uint32_t seed = seedAssist ^ SkScalarRoundToInt(meas.getLength());

    LCGRandom rand(seed ^ ((seed << 16) | (seed >> 16)));
    SkScalar scale = maxDev;
    SkPoint p;
    SkVector v;
    if(smoothness < 0.001f) {
        do {
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
                    Perterb(&p, v, rand.nextSScalar1() * scale);
                    dst->moveTo(p);
                }
                while(--n >= 0) {
                    distance += delta;
                    if(meas.getPosTan(distance, &p, &v)) {
                        Perterb(&p, v, rand.nextSScalar1() * scale);
                        dst->lineTo(p);
                    }
                }
                if(meas.isClosed()) {
                    dst->close();
                }
            }
        } while (meas.nextContour());
    } else {
        SkPoint c1;
        SkPoint c2;
        SkPoint lastP;
        SkPoint nextP;

        do {
            SkScalar smoothLen = smoothness * segLen * 0.4f;
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
                    Perterb(&p, v, rand.nextSScalar1() * scale);
                    dst->moveTo(p);
                    c1 = p;
                }
                while(--n >= 0) {
                    distance += delta;
                    if(meas.getPosTan(distance, &p, &v)) {
                        Perterb(&p, v, rand.nextSScalar1() * scale);
                        SkPoint newC1 = p;
                        makeTangent(&newC1, v, smoothLen);
                        c2 = p;
                        v.negate();
                        makeTangent(&c2, v, smoothLen);

                        dst->cubicTo(c1, c2, p);
                        c1 = newC1;
                    }
                }
                if(meas.isClosed()) {
                    dst->close();
                }
            }
        } while (meas.nextContour());
    }
    return true;
}

void DisplacePathEffect::filterPath(const SkPath &src,
                                    SkPath *dst) {
    displaceFilterPath(dst, src,
                       mMaxDev->qra_getCurrentValue(),
                       mSegLength->qra_getCurrentValue(),
                       mSmoothness->qra_getCurrentValue(),
                       mSeedAssist);
}
