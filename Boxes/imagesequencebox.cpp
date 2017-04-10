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
    schedulePixmapReload();
    mRenderCacheHandler.setupRenderRangeforAnimationRange();
}

void ImageSequenceBox::setUpdateVars() {
    AnimationBox::setUpdateVars();
    mUpdateFramePath = mListOfFrames.at(mUpdateAnimationFrame);
}

void ImageSequenceBox::prp_makeDuplicate(Property *targetBox) {
    AnimationBox::prp_makeDuplicate(targetBox);
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
        mUpdateAnimationImage.load(mUpdateFramePath);
        mUpdateRelBoundingRect = mUpdateAnimationImage.rect();
    } 
}
