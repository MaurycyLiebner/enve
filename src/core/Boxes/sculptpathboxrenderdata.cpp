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

#include "sculptpathboxrenderdata.h"

#include "sculptpathbox.h"
#include "Paint/autotiledsurface.h"
#include "Paint/brushstroke.h"

SculptPathBoxRenderData::SculptPathBoxRenderData(
        SculptPathBox* const parent) :
    BoxRenderData(parent) {}

void SculptPathBoxRenderData::updateRelBoundingRect() {
    if(fPath.isEmpty()) return;
    fRelBoundingRect = fPath.first().boundingRect();
    for(int i = 1; i < fPath.count(); i++) {
        const auto iRect = fPath.at(i).boundingRect();
        fRelBoundingRect = fRelBoundingRect.united(iRect);
    }
}

void SculptPathBoxRenderData::drawSk(SkCanvas * const canvas) {
    Q_UNUSED(canvas)
    if(!fBrush) return;
    if(fPaintSettings.fPaintType != NOPAINT) {
        SkPath path;
        for(const auto& subPath : fPath) {
            const auto& nodes = subPath.nodes();
            bool first = true;
            for(const auto& node : nodes) {
                const SkPoint pos = toSkPoint(node->pos());
                if(first) {
                    path.moveTo(pos);
                    first = false;
                } else path.lineTo(pos);
            }
        }
        path.setFillType(fFillType);
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        fPaintSettings.applyPainterSettingsSk(&paint);
        canvas->drawPath(path, paint);
    }

    AutoTiledSurface surf;
    surf.setPixelClamp(fMaxBoundsRect.translated(-fGlobalRect.topLeft()));
    surf.loadBitmap(mBitmap);

    SkPath pathT;
    QMatrix transform;
    transform.translate(-fGlobalRect.x(), -fGlobalRect.y());
    transform = fScaledTransform*transform;

    const auto strokeBrush = fBrush->getBrush();
    const auto width = fWidth*fResolution;
    for(const auto& subPath : fPath) {
        const auto set = subPath.generateBrushSet(transform, width);
        surf.execute(strokeBrush, set);
    }

    mBitmap.reset();

    QRect baseRect = fGlobalRect;
    baseRect.translate(-surf.zeroTilePos());
    const auto pixRect = surf.pixelBoundingRect();
    baseRect.setSize(QSize(pixRect.width(), pixRect.height()));
    setBaseGlobalRect(baseRect);

    const QMargins iMargins(baseRect.left() - fGlobalRect.left(),
                            baseRect.top() - fGlobalRect.top(),
                            fGlobalRect.right() - baseRect.right(),
                            fGlobalRect.bottom() - baseRect.bottom());
    mBitmap = surf.toBitmap(iMargins);
}
