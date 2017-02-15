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
    void drawSelected(QPainter *p,
                      const CanvasMode &);
    void updateBoundingRect();
    bool relPointInsidePath(QPointF point);
private:
    QPixmap mPixmap;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
