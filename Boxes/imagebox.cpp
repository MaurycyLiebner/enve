#include "Boxes/imagebox.h"
#include <QFileDialog>
#include "mainwindow.h"
#include "filesourcescache.h"

ImageBox::ImageBox(QString filePath) :
    BoundingBox(TYPE_IMAGE) {
    setName("Image");

    setFilePath(filePath);
}


void ImageBox::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    ImageBox *imgTarget = (ImageBox*)targetBox;
    imgTarget->setFilePath(mImageFilePath);
}

BoundingBox *ImageBox::createNewDuplicate() {
    return new ImageBox();
}

void ImageBox::reloadPixmap() {
    mImgCacheHandler->clearCache();

    scheduleUpdate();
}

void ImageBox::setFilePath(QString path) {
    mImageFilePath = path;
    mImgCacheHandler = (ImageCacheHandler*)
                                FileSourcesCache::getHandlerForFilePath(
                                                        path);
    if(mImgCacheHandler == NULL) {
        mImgCacheHandler = new ImageCacheHandler(path);
    }
    reloadPixmap();
}

void ImageBox::addActionsToMenu(QMenu *menu) {
    menu->addAction("Reload")->setObjectName("ib_reload");
    menu->addAction("Set Source File...")->
            setObjectName("ib_set_src_file");
}

void ImageBox::changeSourceFile() {
    MainWindow::getInstance()->disableEventFilter();
    QString importPath = QFileDialog::getOpenFileName(
                                            MainWindow::getInstance(),
                                            "Change Source", "",
                                            "Image Files (*.png *.jpg)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPath.isEmpty()) {
        setFilePath(importPath);
    }
}

void ImageBox::setupBoundingBoxRenderDataForRelFrame(
                                    const int &relFrame,
                                    BoundingBoxRenderData *data) {
    BoundingBox::setupBoundingBoxRenderDataForRelFrame(relFrame, data);
    data->transform.scale(data->resolution, data->resolution);
    ImageBoxRenderData *imgData = (ImageBoxRenderData*)data;
    imgData->image = mImgCacheHandler->getImage();
    if(imgData->image == NULL) {
        mImgCacheHandler->addScheduler();
        mImgCacheHandler->addDependent(imgData);
    }
}

BoundingBoxRenderData *ImageBox::createRenderData() {
    return new ImageBoxRenderData(mImgCacheHandler, this);
}

bool ImageBox::handleSelectedCanvasAction(QAction *selectedAction) {
    if(selectedAction->objectName() == "ib_set_src_file") {
        changeSourceFile();
    } else if(selectedAction->objectName() == "ib_reload") {
        reloadPixmap();
    } else {
        return false;
    }
    return true;
}
#include "filesourcescache.h"
void ImageBoxRenderData::loadImageFromHandler() {
    image = ((ImageCacheHandler*)srcCacheHandler)->getImage();
}
