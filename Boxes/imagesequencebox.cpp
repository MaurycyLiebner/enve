#include "imagesequencebox.h"
#include "filesourcescache.h"

ImageSequenceBox::ImageSequenceBox() :
    AnimationBox() {
    setName("Image Sequence");
}

void ImageSequenceBox::setListOfFrames(const QStringList &listOfFrames) {
    mListOfFrames = listOfFrames;
    reloadFile();
}

void ImageSequenceBox::reloadFile() {
    mAnimationCacheHandler = new ImageSequenceCacheHandler(mListOfFrames);
    AnimationBox::reloadFile();
}

void ImageSequenceBox::makeDuplicate(Property *targetBox) {
    AnimationBox::makeDuplicate(targetBox);
    ImageSequenceBox *animationBoxTarget = (ImageSequenceBox*)targetBox;
    animationBoxTarget->setListOfFrames(mListOfFrames);
    animationBoxTarget->duplicateAnimationBoxAnimatorsFrom(
                mTimeScaleAnimator.data());
}
#include "mainwindow.h"
#include <QFileDialog>
void ImageSequenceBox::changeSourceFile() {
    MainWindow::getInstance()->disableEventFilter();
    QStringList importPaths = QFileDialog::getOpenFileNames(
                MainWindow::getInstance(), "Import Image Sequence",
                "", "Images (*.png *.jpg)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPaths.isEmpty()) {
        setListOfFrames(importPaths);
    }
}

BoundingBox *ImageSequenceBox::createNewDuplicate() {
    return new ImageSequenceBox();
}
