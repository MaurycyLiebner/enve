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

#include "imagerenderdata.h"

ImageRenderData::ImageRenderData(BoundingBox * const parentBoxT) :
    BoxRenderData(parentBoxT) {
    mDelayDataSet = true;
}

void ImageRenderData::updateRelBoundingRect() {
    if(fImage) fRelBoundingRect =
            QRectF(0, 0, fImage->width(), fImage->height());
    else fRelBoundingRect = QRectF(0, 0, 0, 0);
}

void ImageRenderData::setupRenderData() {
    if(!fImage) loadImageFromHandler();
    if(!hasEffects()) setupDirectDraw();
}

void ImageRenderData::setupDirectDraw() {
    fBaseMargin = QMargins();
    updateRelBoundingRect();
    updateGlobalRect();
    fRenderTransform.reset();
    fRenderTransform.translate(fRelBoundingRect.x(), fRelBoundingRect.y());
    fRenderTransform *= fScaledTransform;
    fRenderTransform.translate(-fGlobalRect.x(), -fGlobalRect.y());
    fUseRenderTransform = true;
    fRenderedImage = fImage;
    fAntiAlias = true;
    finishedProcessing();
}

void ImageRenderData::drawSk(SkCanvas * const canvas) {
    SkPaint paint;
    if(fFilterQuality > kNone_SkFilterQuality) paint.setAntiAlias(true);
    paint.setFilterQuality(fFilterQuality);
    if(fImage) canvas->drawImage(fImage,
                                 static_cast<float>(fRelBoundingRect.x()),
                                 static_cast<float>(fRelBoundingRect.y()),
                                 &paint);
}
