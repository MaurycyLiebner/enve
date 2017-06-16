#include "imagesequencebox.h"

ImageSequenceBox::ImageSequenceBox(BoxesGroup *parent) :
    AnimationBox(parent) {
    setName("Image Sequence");
}

void ImageSequenceBox::setListOfFrames(const QStringList &listOfFrames) {
    mListOfFrames = listOfFrames;
    reloadFile();
}

void ImageSequenceBox::reloadFile() {
    mFramesCount = mListOfFrames.count();
    updateDurationRectangleAnimationRange();
    scheduleSoftUpdate();
}

void ImageSequenceBox::setUpdateVars() {
    AnimationBox::setUpdateVars();
    mUpdateFramePath = mListOfFrames.at(mUpdateAnimationFrame);
}

void ImageSequenceBox::makeDuplicate(Property *targetBox) {
    AnimationBox::makeDuplicate(targetBox);
    ImageSequenceBox *animationBoxTarget = (ImageSequenceBox*)targetBox;
    animationBoxTarget->setListOfFrames(mListOfFrames);
    animationBoxTarget->duplicateAnimationBoxAnimatorsFrom(
                mTimeScaleAnimator.data());
}

BoundingBox *ImageSequenceBox::createNewDuplicate(BoxesGroup *parent) {
    return new ImageSequenceBox(parent);
}

void ImageSequenceBox::loadUpdatePixmap() {
    if(mUpdateFramePath.isEmpty()) {
    } else {
        sk_sp<SkData> data = SkData::MakeFromFileName(
                    mUpdateFramePath.toLocal8Bit().data());
        mUpdateAnimationImageSk = SkImage::MakeFromEncoded(data);

        updateUpdateRelBoundingRectFromImage();
    } 
}
