#include "Boxes/imagebox.h"

ImageBox::ImageBox(BoxesGroup *parent, QString filePath) :
    BoundingBox(parent, TYPE_IMAGE)
{
    mImageFilePath = filePath;

    setName("Image");

    reloadPixmap();
}

void ImageBox::updateRelBoundingRect() {
    mRelBoundingRect = mImage.rect();

    BoundingBox::updateRelBoundingRect();
}

bool ImageBox::relPointInsidePath(const QPointF &point)
{
    return mImage.rect().contains(point.toPoint()
                                   );
}

void ImageBox::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    ImageBox *imgTarget = (ImageBox*)targetBox;
    imgTarget->setFilePath(mImageFilePath);
}

BoundingBox *ImageBox::createNewDuplicate(BoxesGroup *parent) {
    return new ImageBox(parent);
}

void ImageBox::draw(QPainter *p) {
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->drawImage(0, 0, mImage);
}

void ImageBox::reloadPixmap()
{
    if(mImageFilePath.isEmpty()) {
    } else {
        mImage.load(mImageFilePath);
    }

    if(!mPivotChanged) centerPivotPosition();
    scheduleSoftUpdate();
}

void ImageBox::setFilePath(QString path)
{
    mImageFilePath = path;
    reloadPixmap();
}
