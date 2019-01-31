#include "Boxes/imagebox.h"
#include <QFileDialog>
#include <QMenu>
#include "GUI/mainwindow.h"
#include "filesourcescache.h"
#include "FileCacheHandlers/imagecachehandler.h"

ImageBox::ImageBox() :
    BoundingBox(TYPE_IMAGE) {
    setName("Image");
}

ImageBox::ImageBox(const QString &filePath) :
    ImageBox() {
    setFilePath(filePath);
}

ImageBox::~ImageBox() {
    if(mImgCacheHandler) {
        mImgCacheHandler->removeDependentBox(this);
    }
}

void ImageBox::setFilePath(const QString &path) {
    mImageFilePath = path;
    if(mImgCacheHandler) {
        mImgCacheHandler->removeDependentBox(this);
    }
    auto handlerT = FileSourcesCache::getHandlerForFilePath(path);
    mImgCacheHandler = GetAsPtr(handlerT, ImageCacheHandler);

    if(!mImgCacheHandler) {
        QFile file(path);
        if(file.exists()) {
            mImgCacheHandler = FileSourcesCache::
                    createNewHandler<ImageCacheHandler>(path);
        }
    } else {
        mImgCacheHandler->setVisibleInListWidgets(true);
    }
    prp_setName(path.split("/").last());
    mImgCacheHandler->addDependentBox(this);
    prp_updateInfluenceRangeAfterChanged();
}

void ImageBox::addActionsToMenu(QMenu *menu) {
    menu->addAction("Reload")->setObjectName("ib_reload");
    menu->addAction("Set Source File...")->
            setObjectName("ib_set_src_file");
}

void ImageBox::changeSourceFile(QWidget* dialogParent) {
    QString importPath = QFileDialog::getOpenFileName(
                                            dialogParent,
                                            "Change Source", "",
                                            "Image Files (*.png *.jpg)");
    if(!importPath.isEmpty()) {
        setFilePath(importPath);
    }
}

void ImageBox::setupBoundingBoxRenderDataForRelFrameF(
                                    const qreal &relFrame,
                                    BoundingBoxRenderData* data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
    auto imgData = GetAsPtr(data, ImageBoxRenderData);
    imgData->fImage = mImgCacheHandler->getImageCopy();
    if(!imgData->fImage) {
        mImgCacheHandler->scheduleTask();
        mImgCacheHandler->addDependent(imgData);
    }
}

stdsptr<BoundingBoxRenderData> ImageBox::createRenderData() {
    return SPtrCreate(ImageBoxRenderData)(mImgCacheHandler, this);
}

bool ImageBox::handleSelectedCanvasAction(QAction *selectedAction,
                                          QWidget* widgetsParent) {
    if(selectedAction->objectName() == "ib_set_src_file") {
        changeSourceFile(widgetsParent);
    } else if(selectedAction->objectName() == "ib_reload") {
        if(mImgCacheHandler) {
            mImgCacheHandler->clearCache();
        }
    } else {
        return false;
    }
    return true;
}
#include "filesourcescache.h"
void ImageBoxRenderData::loadImageFromHandler() {
    fImage = GetAsPtr(srcCacheHandler, ImageCacheHandler)->getImageCopy();
}
