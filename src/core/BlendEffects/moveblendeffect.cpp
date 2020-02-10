// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "moveblendeffect.h"

#include "Boxes/layerboxrenderdata.h"
#include "Boxes/boundingbox.h"

MoveBlendEffect::MoveBlendEffect() :
    BlendEffect(BlendEffectType::move) {
    mZIndex = enve::make_shared<IntAnimator>("z-index");
    ca_addChild(mZIndex);
}

void MoveBlendEffect::blendSetup(
        ChildRenderData &data,
        const int index,
        const qreal relFrame,
        QList<ChildRenderData> &delayed) const {
    const int dIndex = zIndex(relFrame);
    if(dIndex == 0) return;
    const int zIndex = index + (qAbs(dIndex) == 1 ? 2*dIndex : dIndex);
    ChildRenderData iData(data.fData);
    auto& iClip = iData.fClip;
    iClip.fTargetIndex = zIndex;
    if(!isPathValid()) {
        data.fClip.fClipOps.append({SkPath(), SkClipOp::kIntersect, false});
    } else {
        const auto clipPath = this->clipPath(relFrame);
        data.fClip.fClipOps.append({clipPath, SkClipOp::kDifference, false});
        iClip.fClipOps.append({clipPath, SkClipOp::kIntersect, false});
    }

    delayed << iData;
}

void MoveBlendEffect::drawBlendSetup(BoundingBox* const boxToDraw,
                                     const qreal relFrame,
                                     SkCanvas * const canvas,
                                     const SkFilterQuality filter,
                                     const int drawId,
                                     QList<Delayed> &delayed) const {
    const int dIndex = zIndex(relFrame);
    if(dIndex <= 0) return;
    const int zIndex = drawId + (qAbs(dIndex) == 1 ? 2*dIndex : dIndex);
    if(!isPathValid()) {
        delayed << [boxToDraw, zIndex, canvas, filter]
                   (const int drawId, BoundingBox*, BoundingBox*) {
            if(drawId < zIndex) return false;
            canvas->save();
            boxToDraw->drawPixmapSk(canvas, filter);
            canvas->restore();
            return true;
        };
        return;
    }
    const auto clipPath = this->clipPath(relFrame);

    canvas->clipPath(clipPath, SkClipOp::kDifference, false);

    delayed << [boxToDraw, zIndex, clipPath, canvas, filter]
               (const int drawId, BoundingBox*, BoundingBox*) {
        if(drawId < zIndex) return false;
        canvas->save();
        canvas->clipPath(clipPath, SkClipOp::kIntersect, false);
        boxToDraw->drawPixmapSk(canvas, filter);
        canvas->restore();
        return true;
    };
}

int MoveBlendEffect::zIndex(const qreal relFrame) const {
    return mZIndex->getEffectiveIntValue(relFrame);
}
