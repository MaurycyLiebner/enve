#include "subdividepatheffect.h"
#include "Animators/intanimator.h"

SubdividePathEffect::SubdividePathEffect() :
    PathEffect("subdivide effect", PathEffectType::SUBDIVIDE) {
    mCount = enve::make_shared<IntAnimator>(1, 0, 9, 1, "count");
    ca_addChild(mCount);
}

void SubdividePathEffect::apply(const qreal relFrame,
                                const SkPath &src,
                                SkPath * const dst) {
    const int count = mCount->getEffectiveIntValue(relFrame);
    auto lists = CubicList::sMakeFromSkPath(src);
    for(auto & list : lists) {
        list.subdivide(count);
        dst->addPath(list.toSkPath());
    }
}
