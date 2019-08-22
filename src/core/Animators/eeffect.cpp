#include "eeffect.h"

eEffect::eEffect(const QString &name) :
    StaticComplexAnimator(name) {}

void eEffect::writeProperty(eWriteStream& dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst << mVisible;
}

void eEffect::readProperty(eReadStream& src) {
    StaticComplexAnimator::readProperty(src);
    bool visible; src >> visible;
    setVisible(visible);
}

void eEffect::switchVisible() {
    setVisible(!mVisible);
}

void eEffect::setVisible(const bool visible) {
    if(visible == mVisible) return;
    mVisible = visible;
    prp_afterWholeInfluenceRangeChanged();
}

bool eEffect::isVisible() const {
    return mVisible;
}
