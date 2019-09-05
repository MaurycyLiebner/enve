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

#include "sumpatheffect.h"
#include "pathoperations.h"
#include "Boxes/pathbox.h"

SumPathEffect::SumPathEffect() :
    PathEffect("sum effect", PathEffectType::SUM) {
}

class SumEffectCaller : public PathEffectCaller {
public:
    SumEffectCaller() {}

    void apply(SkPath& path);
private:
};

void SumEffectCaller::apply(SkPath &path) {
    QList<SkPath> paths = gBreakApart(path);
    path.reset();
    SkOpBuilder builder;
    for(const auto &subPath : paths) {
        builder.add(subPath, SkPathOp::kUnion_SkPathOp);
    }
    builder.resolve(&path);
}

stdsptr<PathEffectCaller> SumPathEffect::getEffectCaller(const qreal relFrame) const {
    Q_UNUSED(relFrame);
    return enve::make_shared<SumEffectCaller>();
}
