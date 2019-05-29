#include "subdividepatheffect.h"
#include "Animators/intanimator.h"

SubdividePathEffect::SubdividePathEffect(const bool &outlinePathEffect) :
    PathEffect("subdivide effect", SUBDIVIDE_PATH_EFFECT, outlinePathEffect) {
    mCount = SPtrCreate(IntAnimator)(1, 0, 9, 1, "count");
    ca_addChildAnimator(mCount);
}

void SubdividePathEffect::apply(const qreal &relFrame,
                                const SkPath &src,
                                SkPath * const dst) {
    const int count = mCount->getEffectiveIntValue(relFrame);
    auto lists = CubicList::sMakeFromSkPath(src);
    for(auto & list : lists) {
        list.subdivide(count);
        dst->addPath(list.toSkPath());
    }
}

void SubdividePathEffect::writeProperty(QIODevice * const target) const {
    PathEffect::writeProperty(target);
    mCount->writeProperty(target);
}

void SubdividePathEffect::readProperty(QIODevice *target) {
    PathEffect::readProperty(target);
    mCount->readProperty(target);
}
