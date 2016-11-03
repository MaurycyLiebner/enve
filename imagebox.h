#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "boundingbox.h"
#include <QPixmap>

class ImageBox : public BoundingBox
{
public:
    ImageBox(BoxesGroup *parent, QString filePath);

    QRectF getPixBoundingRect();
    void draw(QPainter *p);
    void reloadPixmap();
    void setFilePath(QString path);
    void centerPivotPosition();
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);
    bool absPointInsidePath(QPointF point);
private:
    QPixmap mPixmap;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
