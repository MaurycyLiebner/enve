#include "Boxes/imagebox.h"

ImageBox::ImageBox(BoxesGroup *parent, QString filePath) :
    BoundingBox(parent, TYPE_IMAGE) {
    mImageFilePath = filePath;

    setName("Image");

    reloadPixmap();
}

void ImageBox::updateRelBoundingRect() {
    mRelBoundingRect = QRectF(0., 0.,
                              mImageSk->width(), mImageSk->height());

    BoundingBox::updateRelBoundingRect();
}

void ImageBox::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    ImageBox *imgTarget = (ImageBox*)targetBox;
    imgTarget->setFilePath(mImageFilePath);
}

BoundingBox *ImageBox::createNewDuplicate(BoxesGroup *parent) {
    return new ImageBox(parent);
}

void ImageBox::drawSk(SkCanvas *canvas) {
    SkPaint paint;
    //paint.setFilterQuality(kHigh_SkFilterQuality);
    canvas->drawImage(mImageSk, 0, 0, &paint);
}

void ImageBox::reloadPixmap() {
    if(mImageFilePath.isEmpty()) {
    } else {
        sk_sp<SkData> data = SkData::MakeFromFileName(
                    mImageFilePath.toLocal8Bit().data());
        mImageSk = SkImage::MakeFromEncoded(data);
    }

    scheduleSoftUpdate();
}

void ImageBox::setFilePath(QString path) {
    mImageFilePath = path;
    reloadPixmap();
}
