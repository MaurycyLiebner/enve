#include "patheffect.h"
#include "pointhelpers.h"
#include "skia/skqtconversions.h"
#include "Properties/boolproperty.h"
#include <QDrag>

PathEffect::PathEffect(const QString &name,
                       const PathEffectType type) :
    StaticComplexAnimator(name) {
    mPathEffectType = type;
}

void PathEffect::writeProperty(QIODevice * const dst) const {
    StaticComplexAnimator::writeProperty(dst);
    dst->write(rcConstChar(&mVisible), sizeof(bool));
}

void PathEffect::readProperty(QIODevice * const src) {
    StaticComplexAnimator::readProperty(src);
    src->read(rcChar(&mVisible), sizeof(bool));
    bool tmp;
    src->read(rcChar(&tmp), sizeof(bool));
}

void PathEffect::prp_startDragging() {
    const auto mimeData = new PathEffectMimeData(this);

    const auto drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

void PathEffect::writeIdentifier(QIODevice * const dst) const {
    dst->write(rcConstChar(&mPathEffectType), sizeof(PathEffectType));
}

PathEffectType PathEffect::getEffectType() {
    return mPathEffectType;
}

QMimeData *PathEffect::SWT_createMimeData() {
    return new PathEffectMimeData(this);
}

bool PathEffect::SWT_isPathEffect() const { return true; }

void PathEffect::switchVisible() {
    setVisible(!mVisible);
}

void PathEffect::setVisible(const bool bT) {
    if(bT == mVisible) return;
    mVisible = bT;
    prp_afterWholeInfluenceRangeChanged();
}

bool PathEffect::isVisible() const {
    return mVisible;
}
