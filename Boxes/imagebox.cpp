#include "Boxes/imagebox.h"

ImageBox::ImageBox(BoxesGroup *parent, QString filePath) :
    BoundingBox(parent, TYPE_IMAGE)
{
    mImageFilePath = filePath;

    setName("Image");

    reloadPixmap();
}

void ImageBox::updateBoundingRect() {
    mRelBoundingRect = mPixmap.rect();
    qreal effectsMargin = mEffectsMargin*mUpdateCanvasTransform.m11();
    mPixBoundingRect = mUpdateTransform.mapRect(mRelBoundingRect).
                        adjusted(-effectsMargin, -effectsMargin,
                                 effectsMargin, effectsMargin);

    BoundingBox::updateBoundingRect();
}

void ImageBox::drawSelected(QPainter *p, CanvasMode)
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
    return mPixmap.rect().contains(point.toPoint()
                                   );
}

void ImageBox::draw(QPainter *p)
{
    if(mVisible) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->drawPixmap(0, 0, mPixmap);
    }
}

void ImageBox::reloadPixmap()
{
    if(mImageFilePath.isEmpty()) {
    } else {
        mPixmap.load(mImageFilePath);
    }

    if(!mPivotChanged) centerPivotPosition();
}

void ImageBox::setFilePath(QString path)
{
    mImageFilePath = path;
    reloadPixmap();
}
