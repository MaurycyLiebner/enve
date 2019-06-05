#ifndef SKIAHELPERS_H
#define SKIAHELPERS_H

#include "skiaincludes.h"
#include "exceptions.h"
#include <QIODevice>
#include "castmacros.h"

namespace SkiaHelpers {
    sk_sp<SkImage> makeCopy(const sk_sp<SkImage>& img);
    SkBitmap makeCopy(const SkBitmap& btmp);

    void drawImageGPU(SkCanvas* const canvas,
                      const sk_sp<SkImage>& image,
                      const SkScalar x,
                      const SkScalar y,
                      SkPaint * const paint,
                      GrContext* const context);
    SkImageInfo getPremulBGRAInfo(const int width,
                                  const int height);
    void saveImage(const QString& fileName,
                   const sk_sp<SkImage>& img,
                   const SkEncodedImageFormat& format = SkEncodedImageFormat::kPNG,
                   const int quality = 100);

    sk_sp<SkImage> transferDataToSkImage(SkBitmap& bitmap);

    void writeImg(const sk_sp<SkImage>& img, QIODevice * const file);

    sk_sp<SkImage> readImg(QIODevice * const file);

    void drawOutlineOverlay(SkCanvas * const canvas,
                            const SkPath &path,
                            const SkScalar invScale);

    void drawOutlineOverlay(SkCanvas * const canvas,
                            const SkPath &path,
                            const SkScalar invScale,
                            const SkMatrix& transform);

    void drawOutlineOverlay(SkCanvas * const canvas,
                            const SkPath &path,
                            const SkScalar invScale,
                            const bool dashes,
                            const SkScalar intervalSize);

    void drawOutlineOverlay(SkCanvas * const canvas,
                            const SkPath &path,
                            const SkScalar invScale,
                            const SkMatrix& transform,
                            const bool dashes,
                            const SkScalar intervalSize);
}

#endif // SKIAHELPERS_H
