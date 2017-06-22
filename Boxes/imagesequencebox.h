#ifndef IMAGESEQUENCEBOX_H
#define IMAGESEQUENCEBOX_H
#include "animationbox.h"

class ImageSequenceBox : public AnimationBox{
public:
    ImageSequenceBox(BoxesGroup *parent);
    void setListOfFrames(const QStringList &listOfFrames);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    void reloadFile();
    void makeDuplicate(Property *targetBox);

    bool SWT_isImageSequenceBox() { return true; }

    void changeSourceFile();
private:
    QStringList mListOfFrames;
};

#endif // IMAGESEQUENCEBOX_H
