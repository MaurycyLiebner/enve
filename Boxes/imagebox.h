#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include <QImage>

class ImageBox : public BoundingBox
{
public:
    ImageBox(BoxesGroup *parent, QString filePath = "");

    void draw(QPainter *p);
    void reloadPixmap();
    void setFilePath(QString path);
    void updateRelBoundingRect();
    bool relPointInsidePath(const QPointF &point);

    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);

    bool SWT_isImageBox() { return true; }
private:
    QImage mImage;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
