#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include <QImage>
#include "skiaincludes.h"

class ImageBox : public BoundingBox
{
public:
    ImageBox(BoxesGroup *parent, QString filePath = "");

    void draw(QPainter *p);
    void drawSk(SkCanvas *canvas);
    void reloadPixmap();
    void setFilePath(QString path);
    void updateRelBoundingRect();
    bool relPointInsidePath(const QPointF &point);

    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);

    bool SWT_isImageBox() { return true; }
private:
    QImage mImage;
    sk_sp<SkImage> mImageSk;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
