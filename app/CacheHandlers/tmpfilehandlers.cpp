#include "tmpfilehandlers.h"
#include "imagecachecontainer.h"
#include "castmacros.h"
#include "skia/skiahelpers.h"

void ImgTmpFileDataSaver::writeToFile(QIODevice * const file) {
    SkPixmap pix;
    if(!mImage->peekPixels(&pix)) {
        if(!mImage->makeRasterImage()->peekPixels(&pix)) {
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

void ImgTmpFileDataLoader::readFromFile(QIODevice * const file) {
    int width, height;
    file->read(rcChar(&width), sizeof(int));
    file->read(rcChar(&height), sizeof(int));
    SkBitmap btmp;
    const auto info = SkiaHelpers::getPremulBGRAInfo(width, height);
    btmp.allocPixels(info);
    const qint64 readBytes = width*height*4*
            static_cast<qint64>(sizeof(uchar));
    file->read(scChar(btmp.getPixels()), readBytes);
    mImage = SkiaHelpers::transferDataToSkImage(btmp);
}
