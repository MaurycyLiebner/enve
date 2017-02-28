#include "Boxes/imagebox.h"

ImageBox::ImageBox(BoxesGroup *parent, QString filePath) :
    BoundingBox(parent, TYPE_IMAGE)
{
    mImageFilePath = filePath;

    setName("Image");

    reloadPixmap();
}

void ImageBox::updateBoundingRect() {
    mRelBoundingRect = mImage.rect();
    qreal effectsMargin = mEffectsMargin*mUpdateCanvasTransform.m11();
    mPixBoundingRect = mUpdateTransform.mapRect(mRelBoundingRect).
                        adjusted(-effectsMargin, -effectsMargin,
                                 effectsMargin, effectsMargin);

    BoundingBox::updateBoundingRect();
}

void ImageBox::drawSelected(QPainter *p,
                            const CanvasMode &)
{
    if(mVisible) {
        p->save();

//        QPainterPath mapped;
//        mapped.addRect(mPixmap.rect());
//        mapped = mCombinedTransformMatrix.map(mapped);
//        QPen pen = p->pen();
//        p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
//        p->setBrush(Qt::NoBrush);
//        p->drawPath(mapped);
//        p->setPen(pen);

        drawBoundingRect(p);
        p->restore();
    }
}

bool ImageBox::relPointInsidePath(QPointF point)
{
    return mImage.rect().contains(point.toPoint()
                                   );
}

void ImageBox::makeDuplicate(BoundingBox *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    ImageBox *imgTarget = (ImageBox*)targetBox;
    imgTarget->setFilePath(mImageFilePath);
}

BoundingBox *ImageBox::createNewDuplicate(BoxesGroup *parent) {
    return new ImageBox(parent);
}

void ImageBox::draw(QPainter *p)
{
    if(mVisible) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->drawImage(0, 0, mImage);
    }
}

void ImageBox::reloadPixmap()
{
    if(mImageFilePath.isEmpty()) {
    } else {
        mImage.load(mImageFilePath);
    }

    if(!mPivotChanged) centerPivotPosition();
    scheduleAwaitUpdate();
}

void ImageBox::setFilePath(QString path)
{
    mImageFilePath = path;
    reloadPixmap();
}
