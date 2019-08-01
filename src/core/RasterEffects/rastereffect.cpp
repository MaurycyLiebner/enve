#include "rastereffect.h"
#include "Animators/dynamiccomplexanimator.h"
#include "typemenu.h"

RasterEffect::RasterEffect(const QString &name, const RasterEffectType type) :
    eEffect(name), mType(type) {}

void RasterEffect::writeIdentifier(QIODevice * const dst) const {
    dst->write(rcConstChar(&mType), sizeof(RasterEffectType));
}

void RasterEffect::setupTreeViewMenu(PropertyMenu * const menu) {
    menu->addPlainAction("Delete Effect", [this]() {
        const auto parent = getParent<DynamicComplexAnimatorBase<RasterEffect>>();
        parent->removeChild(ref<RasterEffect>());
    });
}

QMimeData *RasterEffect::SWT_createMimeData() {
    return new eMimeData(QList<RasterEffect*>() << this);
}
