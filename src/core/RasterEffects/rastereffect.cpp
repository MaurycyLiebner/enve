#include "rastereffect.h"
#include "Animators/dynamiccomplexanimator.h"
#include "typemenu.h"

RasterEffect::RasterEffect(const QString &name,
                           const HardwareSupport hwSupport,
                           const bool hwInterchangeable,
                           const RasterEffectType type) :
    eEffect(name), mType(type),
    mTypeHwSupport(hwSupport),
    mHwInterchangeable(hwInterchangeable) {
    if(hwInterchangeable ||
       hwSupport == HardwareSupport::cpuOnly ||
       hwSupport == HardwareSupport::gpuOnly) {
        mInstHwSupport = hwSupport;
    } else if(hwSupport == HardwareSupport::cpuPreffered) {
        mInstHwSupport = HardwareSupport::cpuOnly;
    } else if(hwSupport == HardwareSupport::gpuPreffered) {
        mInstHwSupport = HardwareSupport::gpuOnly;
    } else Q_ASSERT(false);
}

void RasterEffect::writeIdentifier(eWriteStream &dst) const {
    dst.write(&mType, sizeof(RasterEffectType));
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
