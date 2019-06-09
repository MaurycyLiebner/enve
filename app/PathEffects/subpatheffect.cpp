#include "subpatheffect.h"

#include "pointhelpers.h"
#include "Animators/qrealanimator.h"
#include "Properties/boolproperty.h"

SubPathEffect::SubPathEffect() :
    PathEffect("sub-path effect", SUB_PATH_EFFECT) {
    mMin = SPtrCreate(QrealAnimator)("min length");
    mMin->setValueRange(-999, 999);
    mMin->setCurrentBaseValue(0);

    mMax = SPtrCreate(QrealAnimator)("max length");
    mMax->setValueRange(-999, 999);
    mMax->setCurrentBaseValue(100);

    ca_addChildAnimator(mMin);
    ca_addChildAnimator(mMax);
}

void SubPathEffect::readLengthEffect(QIODevice * const target) {
    PathEffect::readProperty(target);
    mMin->anim_setRecording(false);
    mMin->setValueRange(0, 100);
    mMin->setCurrentBaseValue(0);

    mMax->readProperty(target);
    SPtrCreate(BoolProperty)("read only")->readProperty(target);
}

void SubPathEffect::apply(const qreal relFrame, const SkPath &src,
                          SkPath * const dst) {
    const qreal minFrac = mMin->getEffectiveValue(relFrame)/100;
    const qreal maxFrac = mMax->getEffectiveValue(relFrame)/100;
    const bool pathWise = true;

    if(isZero6Dec(maxFrac - 1) && isZero6Dec(minFrac)) {
        *dst = src;
        return;
    }

    if(isZero6Dec(maxFrac - minFrac)) {
        dst->reset();
        return;
    }

    auto paths = CubicList::sMakeFromSkPath(src);

    if(pathWise) {
        SkPath result;

        for(int i = 0; i < paths.count(); i++) {
            auto& path = paths[i];
            result.addPath(path.getFragmentUnbound(minFrac, maxFrac).toSkPath());
        }

        *dst = result;
        return;
    } // else

    qreal totalLength = 0;
    for(auto& path : paths) {
        totalLength += path.getTotalLength();
    }
    const qreal minLength = minFrac*totalLength;
    const qreal maxLength = maxFrac*totalLength;

    SkPath result;
    qreal currLen = qFloor(minLength/totalLength)*totalLength;
    bool first = true;
    while(currLen < maxLength) {
        for(int i = 0; i < paths.count(); i++) {
            auto& path = paths[i];
            const qreal pathLen = path.getTotalLength();

            const qreal minRemLen = minLength - currLen;
            const qreal maxRemLen = maxLength - currLen;
            currLen += pathLen;

            if(first) {
                if(currLen > minLength) {
                    first = false;
                    qreal maxFrag;
                    const bool last = currLen + pathLen > maxLength;
                    if(last) {
                        maxFrag = maxRemLen/pathLen;
                    } else maxFrag = 1;
                    result.addPath(path.getFragment(minRemLen/pathLen, maxFrag).toSkPath());
                    if(last) break;
                }
            } else {
                if(currLen > maxLength) {
                    result.addPath(path.getFragment(0, maxRemLen/pathLen).toSkPath());
                    break;
                } else {
                    result.addPath(path.toSkPath());
                }
            }
        }
    }

    *dst = result;
}
