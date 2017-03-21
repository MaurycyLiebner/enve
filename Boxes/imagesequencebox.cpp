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
    updateDurationRectanglePossibleRange();
    schedulePixmapReload();
}

void ImageSequenceBox::setUpdateVars() {
    AnimationBox::setUpdateVars();
    mUpdateFramePath = mListOfFrames.at(mUpdateAnimationFrame);
}

void ImageSequenceBox::makeDuplicate(BoundingBox *targetBox) {
    AnimationBox::makeDuplicate(targetBox);
    ImageSequenceBox *animationBoxTarget = (ImageSequenceBox*)targetBox;
    animationBoxTarget->setListOfFrames(mListOfFrames);
    animationBoxTarget->duplicateAnimationBoxAnimatorsFrom(
                &mTimeScaleAnimator);
}

BoundingBox *ImageSequenceBox::createNewDuplicate(BoxesGroup *parent) {
    return new ImageSequenceBox(parent);
}

void ImageSequenceBox::loadUpdatePixmap() {
    if(mUpdateFramePath.isEmpty()) {
    } else {
        mUpdateAnimationImage.load(mUpdateFramePath);
    }

    if(!mPivotChanged) centerPivotPosition();
}
