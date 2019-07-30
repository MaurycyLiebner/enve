#include "patheffect.h"
#include "pointhelpers.h"
#include "skia/skqtconversions.h"
#include "Properties/boolproperty.h"
#include <QDrag>

PathEffect::PathEffect(const QString &name,
                       const PathEffectType type) :
    eEffect(name), mPathEffectType(type) {}

void PathEffect::writeIdentifier(QIODevice * const dst) const {
    dst->write(rcConstChar(&mPathEffectType), sizeof(PathEffectType));
}

PathEffectType PathEffect::getEffectType() {
    return mPathEffectType;
}

QMimeData *PathEffect::SWT_createMimeData() {
    return new eMimeData(QList<PathEffect*>() << this);
}

bool PathEffect::SWT_isPathEffect() const { return true; }
