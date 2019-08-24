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

#include "skiahelpers.h"
#include "exceptions.h"

sk_sp<SkImage> SkiaHelpers::makeCopy(const sk_sp<SkImage>& img) {
    if(!img) return nullptr;
    SkPixmap pix;
    if(!img->peekPixels(&pix)) return img->makeRasterImage();
    return SkImage::MakeRasterCopy(pix);
}

SkBitmap SkiaHelpers::makeCopy(const SkBitmap& btmp) {
    if(btmp.isNull()) return SkBitmap();
    SkBitmap result;
    result.allocPixels(btmp.info());
    result.writePixels(btmp.pixmap());
    return result;
}

SkImageInfo SkiaHelpers::getPremulRGBAInfo(const int width,
                                           const int height) {
    return SkImageInfo::Make(width, height,
                             kRGBA_8888_SkColorType,
                             kPremul_SkAlphaType,
                             nullptr);
}
#include <QFile>
void SkiaHelpers::saveImage(const QString &fileName,
                            const sk_sp<SkImage> &img,
                            const SkEncodedImageFormat &format,
                            const int quality) {

    const auto data = img->encodeToData(format, quality);
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly)) {
        const auto cData = static_cast<const char*>(data->data());
        const auto cSize = static_cast<qint64>(data->size());
        const qint64 writeResult = file.write(cData, cSize);
        const bool flushResult = file.flush();
        file.close();
        if(writeResult == -1) RuntimeThrow("Error while writing to file " +
                                           fileName);
        if(!flushResult) RuntimeThrow("Error while flushing buffered data to file " +
                                      fileName);
    } else RuntimeThrow("Could not open file " + fileName);
}

sk_sp<SkImage> SkiaHelpers::transferDataToSkImage(SkBitmap &bitmap) {
    if(bitmap.empty()) return sk_sp<SkImage>();
    bitmap.setImmutable();
    const auto result = SkImage::MakeFromBitmap(bitmap);
    bitmap.reset();
    return result;
}

void SkiaHelpers::writeImg(const sk_sp<SkImage> &img,
                           QIODevice * const file) {
    SkPixmap pix;
    if(!img->peekPixels(&pix)) {
        if(!img->makeRasterImage()->peekPixels(&pix)) {
            RuntimeThrow("Could not peek image pixels");
        }
    }
    writePixmap(pix, file);
}

sk_sp<SkImage> SkiaHelpers::readImg(QIODevice * const file) {
    auto btmp = readBitmap(file);
    return SkiaHelpers::transferDataToSkImage(btmp);
}

void SkiaHelpers::writePixmap(const SkPixmap &pix, QIODevice * const file) {
    const int width = pix.width();
    const int height = pix.height();
    file->write(rcConstChar(&width), sizeof(int));
    file->write(rcConstChar(&height), sizeof(int));
    const qint64 writeBytes = width*height*4*
            static_cast<qint64>(sizeof(uchar));
    file->write(rcConstChar(pix.writable_addr()), writeBytes);
}

SkBitmap SkiaHelpers::readBitmap(QIODevice * const file) {
    int width, height;
    file->read(rcChar(&width), sizeof(int));
    file->read(rcChar(&height), sizeof(int));
    SkBitmap btmp;
    const auto info = SkiaHelpers::getPremulRGBAInfo(width, height);
    btmp.allocPixels(info);
    const qint64 readBytes = width*height*4*
            static_cast<qint64>(sizeof(uchar));
    file->read(scChar(btmp.getPixels()), readBytes);
    return btmp;
}

void SkiaHelpers::writeBitmap(const SkBitmap& bitmap, QIODevice * const file) {
    writePixmap(bitmap.pixmap(), file);
}

void SkiaHelpers::drawOutlineOverlay(SkCanvas * const canvas,
                                     const SkPath &path,
                                     const float invScale) {
    drawOutlineOverlay(canvas, path, invScale, false, 0);
}

void SkiaHelpers::drawOutlineOverlay(SkCanvas * const canvas,
                                     const SkPath &path,
                                     const float invScale,
                                     const SkMatrix& transform) {
    drawOutlineOverlay(canvas, path, invScale, transform, false, 0);
}

void SkiaHelpers::drawOutlineOverlay(SkCanvas * const canvas,
                                     const SkPath &path,
                                     const float invScale,
                                     const bool dashes,
                                     const float intervalSize) {
    SkPaint paint;
    if(dashes) {
        const float intervals[2] = {intervalSize*invScale,
                                       intervalSize*invScale};
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
    }
    paint.setAntiAlias(true);
    paint.setStrokeWidth(1.5f*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    canvas->drawPath(path, paint);
    paint.setStrokeWidth(0.75f*invScale);
    paint.setColor(SK_ColorWHITE);
    canvas->drawPath(path, paint);
}

void SkiaHelpers::drawOutlineOverlay(SkCanvas * const canvas,
                                     const SkPath &path,
                                     const float invScale,
                                     const SkMatrix& transform,
                                     const bool dashes,
                                     const float intervalSize) {
    SkPath mappedPath = path;
    mappedPath.transform(transform);
    drawOutlineOverlay(canvas, mappedPath, invScale,
                       dashes, intervalSize);
}

void SkiaHelpers::forceLink() {
#define FORCE_UNDEFINED_SYMBOL(x, nid) \
    auto __fus_fp_ ## nid = &x; Q_UNUSED(__fus_fp_ ## nid)

    FORCE_UNDEFINED_SYMBOL(SkTextUtils::GetPath, 0);
    FORCE_UNDEFINED_SYMBOL(SkOpBuilder::add, 1);
    GrContext::MakeGL();
    GrGLMakeNativeInterface()->validate();
}
