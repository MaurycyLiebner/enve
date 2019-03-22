#include "lengthpatheffect.h"
#include "edge.h"
#include "pointhelpers.h"
#include "Animators/qrealanimator.h"
#include "Properties/boolproperty.h"

LengthPathEffect::LengthPathEffect(const bool &outlinePathEffect) :
    PathEffect("length effect", LENGTH_PATH_EFFECT, outlinePathEffect) {
    mLength = SPtrCreate(QrealAnimator)("segment length");
    mLength->qra_setValueRange(0, 100);
    mLength->qra_setCurrentValue(100);

    mReverse = SPtrCreate(BoolProperty)("reverse");
    mReverse->setValue(false);

    ca_addChildAnimator(mLength);
    ca_addChildAnimator(mReverse);
}

void LengthPathEffect::apply(const qreal &relFrame,
                             const SkPath &src,
                             SkPath * const dst) {
    const qreal lenPer = mLength->getCurrentEffectiveValueAtRelFrame(relFrame);
    if(lenPer < 0.001) {
        dst->reset();
        return;
    }
    if(lenPer > 99.999) {
        *dst = src;
        return;
    }
    const qreal lenFrac = lenPer/100;
    const bool reverse = mReverse->getValue();

    auto paths = CubicList::sMakeFromSkPath(src);
    qreal totalLength = 0;
    for(auto& path : paths) {
        totalLength += path.getTotalLength();
    }
    const qreal targetLength = lenFrac*totalLength;

    SkPath result;
    qreal currLen = 0;
    if(reverse) {
        for(int i = paths.count() - 1; i >= 0; i++) {
            auto& path = paths[i];
            const qreal pathLen = path.getTotalLength();
            if(currLen + pathLen > targetLength) {
                const qreal remLen = targetLength - currLen;
                const qreal remLenFrac = remLen/pathLen;
                const qreal minLenFrac = CLAMP01(1 - remLenFrac);
                result.addPath(path.getFragment(minLenFrac, 1).toSkPath());
                break;
            }
            currLen += pathLen;
            result.addPath(path.toSkPath());
        }
    } else {
        for(int i = 0; i < paths.count(); i++) {
            auto& path = paths[i];
            const qreal pathLen = path.getTotalLength();
            if(currLen + pathLen > targetLength) {
                const qreal remLen = targetLength - currLen;
                result.addPath(path.getFragment(0, remLen/pathLen).toSkPath());
                break;
            }
            currLen += pathLen;
            result.addPath(path.toSkPath());
        }
    }

    *dst = result;
}
