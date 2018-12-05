#ifndef IMAGESEQUENCEBOX_H
#define IMAGESEQUENCEBOX_H
#include "animationbox.h"

class ImageSequenceBox : public AnimationBox {
public:
    ImageSequenceBox();
    void setListOfFrames(const QStringList &listOfFrames);

    bool SWT_isImageSequenceBox() { return true; }

    void changeSourceFile(QWidget* dialogParent);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
private:
    QStringList mListOfFrames;
};

#endif // IMAGESEQUENCEBOX_H
