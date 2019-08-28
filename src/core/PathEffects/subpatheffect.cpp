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

#include "subpatheffect.h"

#include "pointhelpers.h"
#include "Animators/qrealanimator.h"
#include "Properties/boolproperty.h"

SubPathEffect::SubPathEffect() :
    PathEffect("sub-path effect", PathEffectType::SUB) {
    mMin = enve::make_shared<QrealAnimator>("min length");
    mMin->setValueRange(-999, 999);
    mMin->setCurrentBaseValue(0);

    mMax = enve::make_shared<QrealAnimator>("max length");
    mMax->setValueRange(-999, 999);
    mMax->setCurrentBaseValue(100);

    ca_addChild(mMin);
    ca_addChild(mMax);
}

void SubPathEffect::apply(const qreal relFrame, const SkPath &src,
                          SkPath * const dst) {
    const qreal minFrac = mMin->getEffectiveValue(relFrame)/100;
    const qreal maxFrac = mMax->getEffectiveValue(relFrame)/100;
    const bool pathWise = true;

    if(isZero6Dec(maxFrac - 1) && isZero6Dec(minFrac)) {
        *dst = src;
        return;
    }

    if(isZero6Dec(maxFrac - minFrac)) {
        dst->reset();
        return;
    }

    auto paths = CubicList::sMakeFromSkPath(src);

    if(pathWise) {
        SkPath result;

        for(int i = 0; i < paths.count(); i++) {
            auto& path = paths[i];
            result.addPath(path.getFragmentUnbound(minFrac, maxFrac).toSkPath());
        }

        *dst = result;
        return;
    } // else

    qreal totalLength = 0;
    for(auto& path : paths) {
        totalLength += path.getTotalLength();
    }
    const qreal minLength = minFrac*totalLength;
    const qreal maxLength = maxFrac*totalLength;

    SkPath result;
    qreal currLen = qFloor(minLength/totalLength)*totalLength;
    bool first = true;
    while(currLen < maxLength) {
        for(int i = 0; i < paths.count(); i++) {
            auto& path = paths[i];
            const qreal pathLen = path.getTotalLength();

            const qreal minRemLen = minLength - currLen;
            const qreal maxRemLen = maxLength - currLen;
            currLen += pathLen;

            if(first) {
                if(currLen > minLength) {
                    first = false;
                    qreal maxFrag;
                    const bool last = currLen + pathLen > maxLength;
                    if(last) maxFrag = maxRemLen/pathLen;
                    else maxFrag = 1;
                    result.addPath(path.getFragment(minRemLen/pathLen, maxFrag).toSkPath());
                    if(last) break;
                }
            } else {
                if(currLen > maxLength) {
                    result.addPath(path.getFragment(0, maxRemLen/pathLen).toSkPath());
                    break;
                } else {
                    result.addPath(path.toSkPath());
                }
            }
        }
    }

    *dst = result;
}
