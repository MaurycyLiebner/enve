#include "eeffect.h"

eEffect::eEffect(const QString &name) :
    StaticComplexAnimator(name) {}

void eEffect::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst->write(rcConstChar(&mVisible), sizeof(bool));
}

void eEffect::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    bool visible;
    src->read(rcChar(&visible), sizeof(bool));
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
