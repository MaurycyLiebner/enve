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
    if(mSrcFramesCache) {
        mSrcFramesCache->removeDependentBox(this);
    }
    mSrcFramesCache = FileSourcesCache::
            createNewHandler<ImageSequenceCacheHandler>(
                mListOfFrames);
    mSrcFramesCache->addDependentBox(this);

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
