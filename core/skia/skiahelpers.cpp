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

void SkiaHelpers::drawImageGPU(
        SkCanvas* const canvas,
        const sk_sp<SkImage>& image,
        const SkScalar x,
        const SkScalar y,
        SkPaint * const paint,
        GrContext* const context) {
    if(!image) return;
    if(!context) RuntimeThrow("Invalid draw request");
    const auto mipMap = paint->getFilterQuality() > kLow_SkFilterQuality ?
            GrMipMapped::kYes : GrMipMapped::kNo;
    sk_sp<SkImage> texture(image->makeTextureImage(context, nullptr, mipMap));
    if(!texture) RuntimeThrow("Failed to make texture image");
    canvas->drawImage(texture, x, y, paint);
};

SkImageInfo SkiaHelpers::getPremulBGRAInfo(const int width,
                                           const int height) {
    return SkImageInfo::Make(width, height,
                             kBGRA_8888_SkColorType,
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
    const int width = pix.width();
    const int height = pix.height();
    file->write(rcConstChar(&width), sizeof(int));
    file->write(rcConstChar(&height), sizeof(int));
    const qint64 writeBytes = width*height*4*
            static_cast<qint64>(sizeof(uchar));
    file->write(rcConstChar(pix.writable_addr()), writeBytes);
}

sk_sp<SkImage> SkiaHelpers::readImg(QIODevice * const file) {
    int width, height;
    file->read(rcChar(&width), sizeof(int));
    file->read(rcChar(&height), sizeof(int));
    SkBitmap btmp;
    const auto info = SkiaHelpers::getPremulBGRAInfo(width, height);
    btmp.allocPixels(info);
    const qint64 readBytes = width*height*4*
            static_cast<qint64>(sizeof(uchar));
    file->read(scChar(btmp.getPixels()), readBytes);
    return SkiaHelpers::transferDataToSkImage(btmp);
}

void SkiaHelpers::drawOutlineOverlay(SkCanvas * const canvas,
                                     const SkPath &path,
                                     const SkScalar invScale) {
    drawOutlineOverlay(canvas, path, invScale, false, 0);
}

void SkiaHelpers::drawOutlineOverlay(SkCanvas * const canvas,
                                     const SkPath &path,
                                     const SkScalar invScale,
                                     const SkMatrix& transform) {
    drawOutlineOverlay(canvas, path, invScale, transform, false, 0);
}

void SkiaHelpers::drawOutlineOverlay(SkCanvas * const canvas,
                                     const SkPath &path,
                                     const SkScalar invScale,
                                     const bool dashes,
                                     const SkScalar intervalSize) {
    SkPaint paint;
    if(dashes) {
        const SkScalar intervals[2] = {intervalSize*invScale,
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
                                     const SkScalar invScale,
                                     const SkMatrix& transform,
                                     const bool dashes,
                                     const SkScalar intervalSize) {
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
