#ifndef SKIAHELPERS_H
#define SKIAHELPERS_H

#include "skiaincludes.h"
#include "../exceptions.h"
#include <QIODevice>
#include "../castmacros.h"

namespace SkiaHelpers {
    sk_sp<SkImage> makeCopy(const sk_sp<SkImage>& img);
    SkBitmap makeCopy(const SkBitmap& btmp);

    SkImageInfo getPremulRGBAInfo(const int width,
                                  const int height);
    void saveImage(const QString& fileName,
                   const sk_sp<SkImage>& img,
                   const SkEncodedImageFormat& format = SkEncodedImageFormat::kPNG,
                   const int quality = 100);

    sk_sp<SkImage> transferDataToSkImage(SkBitmap& bitmap);

    void writeImg(const sk_sp<SkImage>& img, QIODevice * const file);
    sk_sp<SkImage> readImg(QIODevice * const file);

    SkBitmap readBitmap(QIODevice * const file);
    void writeBitmap(const SkBitmap& bitmap, QIODevice * const file);

    void writePixmap(const SkPixmap& pix, QIODevice * const file);

    void drawOutlineOverlay(SkCanvas * const canvas,
                            const SkPath &path,
                            const float invScale);

    void drawOutlineOverlay(SkCanvas * const canvas,
                            const SkPath &path,
                            const float invScale,
                            const SkMatrix& transform);

    void drawOutlineOverlay(SkCanvas * const canvas,
                            const SkPath &path,
                            const float invScale,
                            const bool dashes,
                            const float intervalSize);

    void drawOutlineOverlay(SkCanvas * const canvas,
                            const SkPath &path,
                            const float invScale,
                            const SkMatrix& transform,
                            const bool dashes,
                            const float intervalSize);

    void forceLink();
}

#endif // SKIAHELPERS_H
