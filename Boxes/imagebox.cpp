#include "Boxes/imagebox.h"
#include <QFileDialog>
#include "mainwindow.h"

ImageBox::ImageBox(BoxesGroup *parent, QString filePath) :
    BoundingBox(parent, TYPE_IMAGE) {
    mImageFilePath = filePath;

    setName("Image");

    reloadPixmap();
}

void ImageBox::updateRelBoundingRect() {
    mRelBoundingRect = QRectF(0., 0.,
                              mImageSk->width(), mImageSk->height());
    mRelBoundingRectSk = QRectFToSkRect(mRelBoundingRect);

    BoundingBox::updateRelBoundingRect();
}

void ImageBox::makeDuplicate(Property *targetBox) {
    BoundingBox::makeDuplicate(targetBox);
    ImageBox *imgTarget = (ImageBox*)targetBox;
    imgTarget->setFilePath(mImageFilePath);
}

BoundingBox *ImageBox::createNewDuplicate(BoxesGroup *parent) {
    return new ImageBox(parent);
}

void ImageBox::reloadPixmap() {
    if(mImageFilePath.isEmpty()) {
    } else {
        sk_sp<SkData> data = SkData::MakeFromFileName(
                    mImageFilePath.toLocal8Bit().data());
        mImageSk = SkImage::MakeFromEncoded(data);
    }

    scheduleSoftUpdate();
}

void ImageBox::setFilePath(QString path) {
    mImageFilePath = path;
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
    imgData->image = mImageSk;
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
