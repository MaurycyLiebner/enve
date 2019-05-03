#include "subpatheffect.h"

#include "pointhelpers.h"
#include "Animators/qrealanimator.h"
#include "Properties/boolproperty.h"

SubPathEffect::SubPathEffect(const bool &outlinePathEffect) :
    PathEffect("sub-path effect", SUB_PATH_EFFECT, outlinePathEffect) {
    mMin = SPtrCreate(QrealAnimator)("min length");
    mMin->setValueRange(0, 100);
    mMin->setCurrentBaseValue(0);

    mMax = SPtrCreate(QrealAnimator)("max length");
    mMax->setValueRange(0, 100);
    mMax->setCurrentBaseValue(0);

    ca_addChildAnimator(mMin);
    ca_addChildAnimator(mMax);
}

void SubPathEffect::readLengthEffect(QIODevice *target) {
    PathEffect::readProperty(target);
    mMin->anim_setRecording(false);
    mMin->setValueRange(0, 100);
    mMin->setCurrentBaseValue(0);

    mMax->readProperty(target);
    SPtrCreate(BoolProperty)("read only")->readProperty(target);
}

void SubPathEffect::apply(const qreal &relFrame, const SkPath &src,
                          SkPath * const dst) {
    const qreal maxPer = mMax->getEffectiveValue(relFrame);
    const qreal minPer = mMin->getEffectiveValue(relFrame);

    if(maxPer - minPer > 99.999) {
        *dst = src;
        return;
    }

    if(maxPer - minPer < 0.001) {
        dst->reset();
        return;
    }

    auto paths = CubicList::sMakeFromSkPath(src);
    qreal totalLength = 0;
    for(auto& path : paths) {
        totalLength += path.getTotalLength();
    }
    const qreal minLength = minPer*totalLength/100;
    const qreal maxLength = maxPer*totalLength/100;

    SkPath result;
    qreal currLen = 0;
    bool first = true;
    for(int i = 0; i < paths.count(); i++) {
        auto& path = paths[i];
        const qreal pathLen = path.getTotalLength();
        if(first) {
            if(currLen + pathLen > minLength) {
                first = false;
                const qreal remLen = minLength - currLen;
                qreal maxFrag;
                const bool last = currLen + pathLen > maxLength;
                if(last) {
                    const qreal maxRemLen = maxLength - currLen;
                    maxFrag = maxRemLen/pathLen;
                } else maxFrag = 1;
                result.addPath(path.getFragment(remLen/pathLen, maxFrag).toSkPath());
                if(last) break;
            }
        } else {
            if(currLen + pathLen > maxLength) {
                const qreal remLen = maxLength - currLen;
                result.addPath(path.getFragment(0, remLen/pathLen).toSkPath());
                break;
            } else {
                result.addPath(path.toSkPath());
            }
        }
        currLen += pathLen;
    }

    *dst = result;
}

void SubPathEffect::writeProperty(QIODevice * const target) const {
    PathEffect::writeProperty(target);
    mMin->writeProperty(target);
    mMax->writeProperty(target);
}

void SubPathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mMin->readProperty(target);
    mMax->readProperty(target);
}
