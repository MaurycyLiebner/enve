#include "Boxes/imagebox.h"

ImageBox::ImageBox(BoxesGroup *parent, QString filePath) :
    BoundingBox(parent, TYPE_IMAGE)
{
    mImageFilePath = filePath;

    setName("Image");

    reloadPixmap();
}

QRectF ImageBox::getPixBoundingRect()
{
    return mCombinedTransformMatrix.mapRect(mPixmap.rect());
}

void ImageBox::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        p->save();

        QPainterPath mapped;
        mapped.addRect(mPixmap.rect());
        mapped = mCombinedTransformMatrix.map(mapped);
        QPen pen = p->pen();
        p->setPen(QPen(QColor(0, 0, 0, 125), 1.f, Qt::DashLine));
        p->setBrush(Qt::NoBrush);
        p->drawPath(mapped);
        p->setPen(pen);

        //drawBoundingRect(p);
        p->restore();
    }
}

bool ImageBox::absPointInsidePath(QPointF point)
{
    QPainterPath mapped;
    mapped.addRect(mPixmap.rect());
    mapped = mCombinedTransformMatrix.map(mapped);
    return mapped.contains(point);
}

void ImageBox::draw(QPainter *p)
{
    if(mVisible) {
        p->save();

        p->setTransform(QTransform(mCombinedTransformMatrix) );
        p->drawPixmap(0, 0, mPixmap);

        p->restore();
    }
}

void ImageBox::reloadPixmap()
{
    mPixmap.load(mImageFilePath);

    if(!mPivotChanged) centerPivotPosition();
}

void ImageBox::setFilePath(QString path)
{
    mImageFilePath = path;
    reloadPixmap();
}

void ImageBox::centerPivotPosition(bool finish)
{
    mTransformAnimator.setPivotWithoutChangingTransformation(mPixmap.rect().bottomRight()*0.5, finish);
}
