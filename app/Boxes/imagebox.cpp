#include "Boxes/imagebox.h"
#include <QFileDialog>
#include <QMenu>
#include "GUI/mainwindow.h"
#include "filesourcescache.h"
#include "FileCacheHandlers/imagecachehandler.h"

ImageBox::ImageBox() : BoundingBox(TYPE_IMAGE) {
    setName("Image");
}

ImageBox::ImageBox(const QString &filePath) : ImageBox() {
    setFilePath(filePath);
}

void ImageBox::writeBoundingBox(QIODevice * const target) {
    BoundingBox::writeBoundingBox(target);
    gWrite(target, mImageFilePath);
}

void ImageBox::readBoundingBox(QIODevice * const target) {
    BoundingBox::readBoundingBox(target);
    QString path;
    gRead(target, path);
    setFilePath(path);
}

void ImageBox::setFilePath(const QString &path) {
    mImageFilePath = path;
    if(mImgCacheHandler) {
        disconnect(mImgCacheHandler, &ImageCacheHandler::pathChanged,
                   this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
    }
    mImgCacheHandler = FileSourcesCache::getHandlerForFilePath<ImageCacheHandler>(path);

    prp_setName(path.split("/").last());
    if(mImgCacheHandler) {
        connect(mImgCacheHandler, &ImageCacheHandler::pathChanged,
               this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
    }
    prp_afterWholeInfluenceRangeChanged();
}

void ImageBox::reload() {
    if(mImgCacheHandler) mImgCacheHandler->clearCache();
}

#include "typemenu.h"
void ImageBox::addActionsToMenu(BoxTypeMenu * const menu) {
    const auto widget = menu->getParentWidget();

    const BoxTypeMenu::PlainOp<ImageBox> reloadOp =
    [](ImageBox * box) {
        box->reload();
    };
    menu->addPlainAction("Reload", reloadOp);

    const BoxTypeMenu::PlainOp<ImageBox> setSrcOp =
    [widget](ImageBox * box) {
        box->changeSourceFile(widget);
    };
    menu->addPlainAction("Set Source File...", setSrcOp);

    BoundingBox::addActionsToMenu(menu);
}

void ImageBox::changeSourceFile(QWidget * const dialogParent) {
    QString importPath = QFileDialog::getOpenFileName(dialogParent,
                                            "Change Source", mImageFilePath,
                                            "Image Files (*.png *.jpg)");
    if(!importPath.isEmpty()) setFilePath(importPath);
}

void ImageBox::setupRenderData(const qreal relFrame,
                               BoundingBoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto imgData = GetAsPtr(data, ImageBoxRenderData);
    if(mImgCacheHandler->hasImage()) {
        imgData->fImage = mImgCacheHandler->getImageCopy();
    } else {
        const auto loader = mImgCacheHandler->scheduleLoad();
        loader->addDependent(imgData);
    }
}

stdsptr<BoundingBoxRenderData> ImageBox::createRenderData() {
    return SPtrCreate(ImageBoxRenderData)(mImgCacheHandler, this);
}

#include "filesourcescache.h"
void ImageBoxRenderData::loadImageFromHandler() {
    fImage = fSrcCacheHandler->getImageCopy();
}
