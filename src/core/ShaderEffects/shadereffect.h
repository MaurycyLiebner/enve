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

#ifndef SHADEREFFECT_H
#define SHADEREFFECT_H
#include "RasterEffects/rastereffect.h"
#include "shadereffectcreator.h"
#include "Tasks/updatable.h"

class ShaderEffect : public RasterEffect {
    e_OBJECT
    ShaderEffect(const QString &name,
                 const ShaderEffectCreator * const creator,
                 const ShaderEffectProgram * const program,
                 const QList<stdsptr<ShaderPropertyCreator>>& props);
public:
    void writeIdentifier(eWriteStream &dst) const;

    stdsptr<RasterEffectCaller> getEffectCaller(
            const qreal relFrame, const qreal resolution,
            const qreal influence) const;

    void updateIfUsesProgram(const ShaderEffectProgram * const program) {
        if(program == mProgram)
            prp_afterWholeInfluenceRangeChanged();
    }
private:
    const ShaderEffectProgram * const mProgram;
    const ShaderEffectCreator * const mCreator;
};

#endif // SHADEREFFECT_H
