#include "patheffect.h"
#include "pointhelpers.h"
#include "skia/skqtconversions.h"
#include "Properties/boolproperty.h"
#include <QDrag>

PathEffect::PathEffect(const QString &name,
                       const PathEffectType type,
                       const bool outlinePathEffect) :
    StaticComplexAnimator(name) {
    mPathEffectType = type;
    mApplyBeforeThickness = SPtrCreate(BoolProperty)("pre-thickness");
    ca_addChildAnimator(mApplyBeforeThickness);
    setIsOutlineEffect(outlinePathEffect);
}

void PathEffect::prp_startDragging() {
    const auto mimeData = new PathEffectMimeData(this);

    const auto drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec();
}

const PathEffectType &PathEffect::getEffectType() {
    return mPathEffectType;
}

bool PathEffect::applyBeforeThickness() {
    if(!mApplyBeforeThickness) return false;
    return mApplyBeforeThickness->getValue();
}

QMimeData *PathEffect::SWT_createMimeData() {
    return new PathEffectMimeData(this);
}

bool PathEffect::SWT_isPathEffect() const { return true; }

void PathEffect::setIsOutlineEffect(const bool bT) {
    if(bT == mOutlineEffect) return;
    mOutlineEffect = bT;
    mApplyBeforeThickness->SWT_setVisible(bT);
}

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

bool PathEffect::hasReasonsNotToApplyUglyTransform() { return false; }
