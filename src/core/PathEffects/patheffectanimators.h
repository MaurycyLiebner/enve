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

#ifndef PATHEFFECTANIMATORS_H
#define PATHEFFECTANIMATORS_H
#include "PathEffects/patheffect.h"
#include "Animators/dynamiccomplexanimator.h"
#include "skia/skiaincludes.h"
class PathBox;
class PathEffectsTask;

qsptr<PathEffect> readIdCreatePathEffect(eReadStream& src);

typedef DynamicComplexAnimator<PathEffect,
        &PathEffect::writeIdentifier,
        &readIdCreatePathEffect> PathEffectAnimatorsBase;
class PathEffectAnimators : public PathEffectAnimatorsBase {
    e_OBJECT
protected:
    PathEffectAnimators();
public:
    bool SWT_isPathEffectAnimators() const;

    bool hasEffects();
    void readPathEffect(eReadStream &target);

    void addEffects(const qreal relFrame,
                    QList<stdsptr<PathEffectCaller>>& list) const;
private:
};


#endif // PATHEFFECTANIMATORS_H
