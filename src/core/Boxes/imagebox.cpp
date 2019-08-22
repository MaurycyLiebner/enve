#include "Boxes/imagebox.h"
#include <QFileDialog>
#include <QMenu>
#include "FileCacheHandlers/imagecachehandler.h"
#include "fileshandler.h"

ImageBox::ImageBox() : BoundingBox(TYPE_IMAGE) {
    prp_setName("Image");
}

ImageBox::ImageBox(const QString &filePath) : ImageBox() {
    setFilePath(filePath);
}

void ImageBox::writeBoundingBox(eWriteStream& dst) {
    BoundingBox::writeBoundingBox(dst);
    dst << mImageFilePath;
}

void ImageBox::readBoundingBox(eReadStream& src) {
    BoundingBox::readBoundingBox(src);
    QString path; src >> path;
    setFilePath(path);
}

void ImageBox::setFilePath(const QString &path) {
    mImageFilePath = path;
    if(mImgCacheHandler) {
        disconnect(mImgCacheHandler, &ImageFileHandler::pathChanged,
                   this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        disconnect(mImgCacheHandler, &ImageFileHandler::reloaded,
                   this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
    }
    mImgCacheHandler = FilesHandler::sInstance->getFileHandler<ImageFileHandler>(path);

    prp_setName(path.split("/").last());
    if(mImgCacheHandler) {
        connect(mImgCacheHandler, &ImageFileHandler::pathChanged,
                this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        connect(mImgCacheHandler, &ImageFileHandler::reloaded,
                this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
    }
    prp_afterWholeInfluenceRangeChanged();
}

void ImageBox::reload() {
    if(mImgCacheHandler) mImgCacheHandler->reloadAction();
}

#include "typemenu.h"
void ImageBox::setupCanvasMenu(PropertyMenu * const menu) {
    const auto widget = menu->getParentWidget();

    const PropertyMenu::PlainSelectedOp<ImageBox> reloadOp =
    [](ImageBox * box) {
        box->reload();
    };
    menu->addPlainAction("Reload", reloadOp);

    const PropertyMenu::PlainSelectedOp<ImageBox> setSrcOp =
    [widget](ImageBox * box) {
        box->changeSourceFile(widget);
    };
    menu->addPlainAction("Set Source File...", setSrcOp);

    BoundingBox::setupCanvasMenu(menu);
}

void ImageBox::changeSourceFile(QWidget * const dialogParent) {
    QString importPath = QFileDialog::getOpenFileName(dialogParent,
                                            "Change Source", mImageFilePath,
                                            "Image Files (*.png *.jpg)");
    if(!importPath.isEmpty()) setFilePath(importPath);
}

void ImageBox::setupRenderData(const qreal relFrame,
                               BoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    const auto imgData = static_cast<ImageBoxRenderData*>(data);
    if(mImgCacheHandler->hasImage()) {
        imgData->fImage = mImgCacheHandler->getImageCopy();
    } else {
        const auto loader = mImgCacheHandler->scheduleLoad();
        loader->addDependent(imgData);
    }
}

stdsptr<BoxRenderData> ImageBox::createRenderData() {
    return enve::make_shared<ImageBoxRenderData>(mImgCacheHandler, this);
}

void ImageBoxRenderData::loadImageFromHandler() {
    fImage = fSrcCacheHandler->getImageCopy();
}
