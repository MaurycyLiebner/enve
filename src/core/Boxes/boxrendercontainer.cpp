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

#include "boxrendercontainer.h"
#include "boxrenderdata.h"
#include "skia/skiahelpers.h"

void RenderContainer::drawSk(SkCanvas * const canvas,
                             SkPaint& paint) const {
    if(!mSrcRenderData) return;
    canvas->save();
    canvas->concat(toSkMatrix(mPaintTransform));
    const auto blendMode = paint.getBlendMode();
    if(blendMode == SkBlendMode::kDstIn ||
       blendMode == SkBlendMode::kSrcIn ||
       blendMode == SkBlendMode::kDstATop ||
       blendMode == SkBlendMode::kModulate ||
       blendMode == SkBlendMode::kSrcOut) {
        canvas->save();
        auto rect = SkRect::MakeXYWH(mGlobalRect.x(), mGlobalRect.y(),
                                     mImageSk->width(), mImageSk->height());
        rect.inset(1, 1);
        canvas->clipRect(rect, SkClipOp::kDifference, false);
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->restore();
    }
    paint.setAntiAlias(mAntiAlias);
    canvas->drawImage(mImageSk, mGlobalRect.x(), mGlobalRect.y(), &paint);
    canvas->restore();
}

void RenderContainer::updatePaintTransformGivenNewTotalTransform(
                                    const QMatrix &totalTransform) {
    mPaintTransform = mTransform.inverted()*totalTransform;
    const qreal invRes = 1/mResolutionFraction;
    mPaintTransform.scale(invRes, invRes);
    mPaintTransform = mRenderTransform*mPaintTransform;
}

void RenderContainer::clear() {
    mImageSk.reset();
    mSrcRenderData.reset();
}

void RenderContainer::setSrcRenderData(BoxRenderData * const data) {
    mTransform = data->fTotalTransform;
    mResolutionFraction = data->fResolution;
    mImageSk = data->fRenderedImage;
    mGlobalRect = data->fGlobalRect;
    mAntiAlias = data->fAntiAlias;
    mPaintTransform.reset();
    mPaintTransform.scale(1/mResolutionFraction, 1/mResolutionFraction);
    mRenderTransform = data->fRenderTransform;
    mPaintTransform = mRenderTransform*mPaintTransform;
    mSrcRenderData = data->ref<BoxRenderData>();
}
