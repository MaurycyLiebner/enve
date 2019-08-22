#ifndef IMAGESEQUENCEBOX_H
#define IMAGESEQUENCEBOX_H
#include "animationbox.h"

class ImageSequenceBox : public AnimationBox {
    e_OBJECT
protected:
    ImageSequenceBox();
public:
    void setFolderPath(const QString &folderPath);

    bool SWT_isImageSequenceBox() const { return true; }

    void changeSourceFile(QWidget * const dialogParent);
    void writeBoundingBox(eWriteStream& dst);
    void readBoundingBox(eReadStream& src);
};

#endif // IMAGESEQUENCEBOX_H
