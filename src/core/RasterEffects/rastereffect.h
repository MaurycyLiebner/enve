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

#ifndef RASTEREFFECT_H
#define RASTEREFFECT_H
#include "../Animators/eeffect.h"
#include "../glhelpers.h"
#include "rastereffectcaller.h"

enum class RasterEffectType : short {
    BLUR,
    SHADOW,
    CUSTOM, // C++
    CUSTOM_SHADER, // xml, GLSL
    TYPE_COUNT
};

class RasterEffect : public eEffect {
    e_OBJECT
    Q_OBJECT
protected:
    RasterEffect(const QString &name,
                 const HardwareSupport hwSupport,
                 const bool hwInterchangeable,
                 const RasterEffectType type);
public:
    virtual stdsptr<RasterEffectCaller> getEffectCaller(
            const qreal relFrame, const qreal resolution) const = 0;

    virtual bool forceMargin() const { return false; }
    virtual QMargins getMargin() const { return QMargins(); }

    void writeIdentifier(eWriteStream& dst) const;

    void setupTreeViewMenu(PropertyMenu * const menu);

    bool SWT_isRasterEffect() const final { return true; }

    QMimeData *SWT_createMimeData() final;

    HardwareSupport instanceHwSupport() const {
        return mInstHwSupport;
    }

    void switchInstanceHwSupport() {
        if(mTypeHwSupport == HardwareSupport::cpuOnly) return;
        if(mTypeHwSupport == HardwareSupport::gpuOnly) return;
        if(mInstHwSupport == HardwareSupport::cpuOnly) {
            if(mHwInterchangeable) mInstHwSupport = mTypeHwSupport;
            else mInstHwSupport = HardwareSupport::gpuOnly;
        } else if(mInstHwSupport == HardwareSupport::gpuOnly) {
            mInstHwSupport = HardwareSupport::cpuOnly;
        } else mInstHwSupport = HardwareSupport::gpuOnly;
        if(!mHwInterchangeable) prp_afterWholeInfluenceRangeChanged();
    }
signals:
    void forcedMarginChanged();
private:
    const RasterEffectType mType;
    const HardwareSupport mTypeHwSupport;
    const bool mHwInterchangeable;
    HardwareSupport mInstHwSupport;
};

#endif // RASTEREFFECT_H
