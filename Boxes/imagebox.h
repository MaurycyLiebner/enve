#ifndef IMAGEBOX_H
#define IMAGEBOX_H
#include "Boxes/boundingbox.h"
#include <QImage>
#include "skiaincludes.h"

class ImageBox : public BoundingBox
{
public:
    ImageBox(BoxesGroup *parent, QString filePath = "");

    void drawSk(SkCanvas *canvas);
    void reloadPixmap();
    void setFilePath(QString path);
    void updateRelBoundingRect();

    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);

    bool SWT_isImageBox() { return true; }
    void addActionsToMenu(QMenu *menu);
    bool handleSelectedCanvasAction(QAction *selectedAction);
private:
    sk_sp<SkImage> mImageSk;
    QString mImageFilePath;
};

#endif // IMAGEBOX_H
