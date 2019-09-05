// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
    const PropertyMenu::PlainSelectedOp<RasterEffect> dOp =
    [](RasterEffect* const eff) {
        const auto parent = eff->getParent<DynamicComplexAnimatorBase<RasterEffect>>();
        parent->removeChild(eff->ref<RasterEffect>());
    };
    menu->addPlainAction("Delete Effect(s)", dOp);
    eEffect::setupTreeViewMenu(menu);
}

QMimeData *RasterEffect::SWT_createMimeData() {
    return new eMimeData(QList<RasterEffect*>() << this);
}
