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

#include "imagecachecontainer.h"
#include "tmpdeleter.h"
#include "canvas.h"
#include "skia/skiahelpers.h"

ImageCacheContainer::ImageCacheContainer(const FrameRange &range,
                                         HddCachableCacheHandler * const parent) :
    HddCachableRangeCont(range, parent) {}

ImageCacheContainer::ImageCacheContainer(const sk_sp<SkImage> &img,
                                         const FrameRange &range,
                                         HddCachableCacheHandler * const parent) :
    ImageCacheContainer(range, parent) {
    replaceImageSk(img);
}

void ImageCacheContainer::replaceImageSk(const sk_sp<SkImage> &img) {
    mImage = img;
    mImageCopies.clear();
    afterDataReplaced();
}

int ImageCacheContainer::getByteCount() {
    if(!mImage) return 0;
    SkPixmap pixmap;
    if(mImage->peekPixels(&pixmap)) {
        const int nCopies = mImageCopies.count();
        return pixmap.width()*pixmap.height()*
               pixmap.info().bytesPerPixel()*(1 + nCopies);
    }
    return 0;
}

sk_sp<SkImage> ImageCacheContainer::getImageSk() {
    return mImage;
}

void ImageCacheContainer::setDataLoadedFromTmpFile(const sk_sp<SkImage> &img) {
    replaceImageSk(img);
    afterDataLoadedFromTmpFile();
}

#include "efiltersettings.h"
void ImageCacheContainer::drawSk(SkCanvas * const canvas,
                                 const SkFilterQuality filter) {
    SkPaint paint;
    paint.setFilterQuality(filter);
    canvas->drawImage(mImage, 0, 0, &paint);
}

int ImageCacheContainer::clearMemory() {
    const int bytes = getByteCount();
    mImage.reset();
    mImageCopies.clear();
    return bytes;
}

stdsptr<eHddTask> ImageCacheContainer::createTmpFileDataSaver() {
    return enve::make_shared<ImgSaver>(this, mImage);
}

stdsptr<eHddTask> ImageCacheContainer::createTmpFileDataLoader() {
    const ImgLoader::Func func = [this](sk_sp<SkImage> img) {
        setDataLoadedFromTmpFile(img);
    };
    return enve::make_shared<ImgLoader>(mTmpFile, this, func);
}

sk_sp<SkImage> ImageCacheContainer::requestCopy() {
    if(mImageCopies.isEmpty()) return SkiaHelpers::makeCopy(mImage);
    else return mImageCopies.takeLast();
}
