#include "skiahelpers.h"

sk_sp<SkImage> SkiaHelpers::makeSkImageCopy(const sk_sp<SkImage>& img) {
    if(!img) return nullptr;
    SkPixmap pix;
    if(!img->peekPixels(&pix)) {
        return img->makeRasterImage();
    }
    return SkImage::MakeRasterCopy(pix);
}

void SkiaHelpers::drawImageGPU(
        SkCanvas* const canvas,
        const sk_sp<SkImage>& image,
        const SkScalar& x,
        const SkScalar& y,
        SkPaint * const paint,
        GrContext* const context) {
    if(!image || !context) return;
    sk_sp<SkImage> texture(image->makeTextureImage(context, nullptr));
    canvas->drawImage(texture, x, y, paint);
};

SkImageInfo SkiaHelpers::getPremulBGRAInfo(const int &width,
                                           const int &height) {
    return SkImageInfo::Make(width, height,
                             kBGRA_8888_SkColorType,
                             kPremul_SkAlphaType,
                             nullptr);
}
