#include "skiahelpers.h"
#include "exceptions.h"

sk_sp<SkImage> SkiaHelpers::makeSkImageCopy(const sk_sp<SkImage>& img) {
    if(!img) return nullptr;
    SkPixmap pix;
    if(!img->peekPixels(&pix)) return img->makeRasterImage();
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
#include <QFile>
void SkiaHelpers::saveImage(const QString &fileName,
                            const sk_sp<SkImage> &img,
                            const SkEncodedImageFormat &format,
                            const int &quality) {

    const auto data = img->encodeToData(format, quality);
    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly)) {
        const auto cData = static_cast<const char*>(data->data());
        const auto cSize = static_cast<qint64>(data->size());
        const qint64 writeResult = file.write(cData, cSize);
        const bool flushResult = file.flush();
        file.close();
        if(writeResult == -1) RuntimeThrow("Error while writing to file " +
                                           fileName.toStdString());
        if(!flushResult) RuntimeThrow("Error while flushing buffered data to file " +
                                      fileName.toStdString());
    } else RuntimeThrow("Could not open file " + fileName.toStdString());
}
