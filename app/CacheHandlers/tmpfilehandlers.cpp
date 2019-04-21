#include "tmpfilehandlers.h"
#include "imagecachecontainer.h"
#include "castmacros.h"
#include "skia/skiahelpers.h"

void ImgTmpFileDataSaver::writeToFile(QIODevice * const file) {
    SkiaHelpers::writeImg(mImage, file);
}

void ImgTmpFileDataLoader::readFromFile(QIODevice * const file) {
    mImage = SkiaHelpers::readImg(file);
}
