#include "imagesequencebox.h"
#include "FileCacheHandlers/imagesequencecachehandler.h"
#include "filesourcescache.h"
#include "GUI/mainwindow.h"
#include <QFileDialog>

ImageSequenceBox::ImageSequenceBox() : AnimationBox(TYPE_IMAGESQUENCE) {
    setName("Image Sequence");
}

void ImageSequenceBox::setListOfFrames(const QStringList &listOfFrames) {
    mListOfFrames = listOfFrames;
//    if(mSrcFramesCache) {
//        mSrcFramesCache->removeDependentBox(this);
//    }
    mSrcFramesCache = SPtrCreate(ImageSequenceCacheHandler)(mListOfFrames);
//    mSrcFramesCache = FileSourcesCache::
//            createNewHandler<ImageSequenceCacheHandler>(mListOfFrames);
//    mSrcFramesCache->addDependentBox(this);

    reloadCacheHandler();
}

void ImageSequenceBox::changeSourceFile(QWidget *dialogParent) {
    QStringList importPaths = QFileDialog::getOpenFileNames(
                dialogParent, "Import Image Sequence",
                "", "Images (*.png *.jpg)");
    if(!importPaths.isEmpty()) {
        setListOfFrames(importPaths);
    }
}

void ImageSequenceBox::writeBoundingBox(QIODevice * const target) {
    AnimationBox::writeBoundingBox(target);
    int nFrames = mListOfFrames.count();
    target->write(rcConstChar(&nFrames), sizeof(int));
    for(const QString &frame : mListOfFrames) {
        gWrite(target, frame);
    }
}

void ImageSequenceBox::readBoundingBox(QIODevice * const target) {
    AnimationBox::readBoundingBox(target);
    int nFrames;
    target->read(rcChar(&nFrames), sizeof(int));
    QStringList frames;
    for(int i = 0; i < nFrames; i++) {
        frames << gReadString(target);
    }
    setListOfFrames(frames);
}
