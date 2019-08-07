#include "imagesequencebox.h"
#include "FileCacheHandlers/imagesequencecachehandler.h"
#include "filesourcescache.h"
#include <QFileDialog>

ImageSequenceBox::ImageSequenceBox() : AnimationBox(TYPE_IMAGESQUENCE) {
    prp_setName("Image Sequence");
}

void ImageSequenceBox::setFolderPath(const QString &folderPath) {
    const auto iscHandler = enve::make_shared<ImageSequenceCacheHandler>();
    iscHandler->setFolderPath(folderPath);
    mSrcFramesCache = iscHandler;

    animationDataChanged();
}

void ImageSequenceBox::changeSourceFile(QWidget * const dialogParent) {
    const auto dir = QFileDialog::getExistingDirectory(
                dialogParent, "Import Image Sequence");
    if(!dir.isEmpty()) setFolderPath(dir);
}

void ImageSequenceBox::writeBoundingBox(QIODevice * const target) {
    AnimationBox::writeBoundingBox(target);

}

void ImageSequenceBox::readBoundingBox(QIODevice * const target) {
    AnimationBox::readBoundingBox(target);
;
}
