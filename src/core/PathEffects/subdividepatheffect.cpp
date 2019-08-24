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

#include "subdividepatheffect.h"
#include "Animators/intanimator.h"

SubdividePathEffect::SubdividePathEffect() :
    PathEffect("subdivide effect", PathEffectType::SUBDIVIDE) {
    mCount = enve::make_shared<IntAnimator>(1, 0, 9, 1, "count");
    ca_addChild(mCount);
}

void SubdividePathEffect::apply(const qreal relFrame,
                                const SkPath &src,
                                SkPath * const dst) {
    const int count = mCount->getEffectiveIntValue(relFrame);
    auto lists = CubicList::sMakeFromSkPath(src);
    for(auto & list : lists) {
        list.subdivide(count);
        dst->addPath(list.toSkPath());
    }
}
