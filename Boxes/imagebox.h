#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include <QPixmap>

class ImageBox : public BoundingBox
{
public:
    ImageBox(BoxesGroup *parent, QString filePath = "");

    void draw(QPainter *p);
    void reloadPixmap();
    void setFilePath(QString path);
    void centerPivotPosition(bool finish = false);
    void drawSelected(QPainter *p, CanvasMode);
    bool absPointInsidePath(QPointF point);
    void updateBoundingRect();
private:
    QPixmap mPixmap;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
