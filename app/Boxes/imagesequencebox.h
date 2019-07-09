#ifndef IMAGESEQUENCEBOX_H
#define IMAGESEQUENCEBOX_H
#include "animationbox.h"

class ImageSequenceBox : public AnimationBox {
    friend class SelfRef;
protected:
    ImageSequenceBox();
public:
    void setListOfFrames(const QStringList &listOfFrames);

    bool SWT_isImageSequenceBox() const { return true; }

    void changeSourceFile(QWidget * const dialogParent);
    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);
private:
    QStringList mListOfFrames;
};

#endif // IMAGESEQUENCEBOX_H
